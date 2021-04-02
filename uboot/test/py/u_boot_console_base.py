# Copyright (c) 2015 Stephen Warren
# Copyright (c) 2015-2016, NVIDIA CORPORATION. All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0

# Common logic to interact with U-Boot via the console. This class provides
# the interface that tests use to execute U-Boot shell commands and wait for
# their results. Sub-classes exist to perform board-type-specific setup
# operations, such as spawning a sub-process for Sandbox, or attaching to the
# serial console of real hardware.

import multiplexed_log
import os
import pytest
import re
import sys

# Regexes for text we expect U-Boot to send to the console.
pattern_u_boot_spl_signon = re.compile('(U-Boot SPL \\d{4}\\.\\d{2}-[^\r\n]*)')
pattern_u_boot_main_signon = re.compile('(U-Boot \\d{4}\\.\\d{2}-[^\r\n]*)')
pattern_stop_autoboot_prompt = re.compile('Hit any key to stop autoboot: ')
pattern_unknown_command = re.compile('Unknown command \'.*\' - try \'help\'')
pattern_error_notification = re.compile('## Error: ')

class ConsoleDisableCheck(object):
    '''Context manager (for Python's with statement) that temporarily disables
    the specified console output error check. This is useful when deliberately
    executing a command that is known to trigger one of the error checks, in
    order to test that the error condition is actually raised. This class is
    used internally by ConsoleBase::disable_check(); it is not intended for
    direct usage.'''

    def __init__(self, console, check_type):
        self.console = console
        self.check_type = check_type

    def __enter__(self):
        self.console.disable_check_count[self.check_type] += 1

    def __exit__(self, extype, value, traceback):
        self.console.disable_check_count[self.check_type] -= 1

class ConsoleBase(object):
    '''The interface through which test functions interact with the U-Boot
    console. This primarily involves executing shell commands, capturing their
    results, and checking for common error conditions. Some common utilities
    are also provided too.'''

    def __init__(self, log, config, max_fifo_fill):
        '''Initialize a U-Boot console connection.

        Can only usefully be called by sub-classes.

        Args:
            log: A mulptiplex_log.Logfile object, to which the U-Boot output
                will be logged.
            config: A configuration data structure, as built by conftest.py.
            max_fifo_fill: The maximum number of characters to send to U-Boot
                command-line before waiting for U-Boot to echo the characters
                back. For UART-based HW without HW flow control, this value
                should be set less than the UART RX FIFO size to avoid
                overflow, assuming that U-Boot can't keep up with full-rate
                traffic at the baud rate.

        Returns:
            Nothing.
        '''

        self.log = log
        self.config = config
        self.max_fifo_fill = max_fifo_fill

        self.logstream = self.log.get_stream('console', sys.stdout)

        # Array slice removes leading/trailing quotes
        self.prompt = self.config.buildconfig['config_sys_prompt'][1:-1]
        self.prompt_escaped = re.escape(self.prompt)
        self.p = None
        self.disable_check_count = {
            'spl_signon': 0,
            'main_signon': 0,
            'unknown_command': 0,
            'error_notification': 0,
        }

        self.at_prompt = False
        self.at_prompt_logevt = None
        self.ram_base = None

    def close(self):
        '''Terminate the connection to the U-Boot console.

        This function is only useful once all interaction with U-Boot is
        complete. Once this function is called, data cannot be sent to or
        received from U-Boot.

        Args:
            None.

        Returns:
            Nothing.
        '''

        if self.p:
            self.p.close()
        self.logstream.close()

    def run_command(self, cmd, wait_for_echo=True, send_nl=True,
            wait_for_prompt=True):
        '''Execute a command via the U-Boot console.

        The command is always sent to U-Boot.

        U-Boot echoes any command back to its output, and this function
        typically waits for that to occur. The wait can be disabled by setting
        wait_for_echo=False, which is useful e.g. when sending CTRL-C to
        interrupt a long-running command such as "ums".

        Command execution is typically triggered by sending a newline
        character. This can be disabled by setting send_nl=False, which is
        also useful when sending CTRL-C.

        This function typically waits for the command to finish executing, and
        returns the console output that it generated. This can be disabled by
        setting wait_for_prompt=False, which is useful when invoking a long-
        running command such as "ums".

        Args:
            cmd: The command to send.
            wait_for_each: Boolean indicating whether to wait for U-Boot to
                echo the command text back to its output.
            send_nl: Boolean indicating whether to send a newline character
                after the command string.
            wait_for_prompt: Boolean indicating whether to wait for the
                command prompt to be sent by U-Boot. This typically occurs
                immediately after the command has been executed.

        Returns:
            If wait_for_prompt == False:
                Nothing.
            Else:
                The output from U-Boot during command execution. In other
                words, the text U-Boot emitted between the point it echod the
                command string and emitted the subsequent command prompts.
        '''

        self.ensure_spawned()

        if self.at_prompt and \
                self.at_prompt_logevt != self.logstream.logfile.cur_evt:
            self.logstream.write(self.prompt, implicit=True)

        bad_patterns = []
        bad_pattern_ids = []
        if (self.disable_check_count['spl_signon'] == 0 and
                self.u_boot_spl_signon):
            bad_patterns.append(self.u_boot_spl_signon_escaped)
            bad_pattern_ids.append('SPL signon')
        if self.disable_check_count['main_signon'] == 0:
            bad_patterns.append(self.u_boot_main_signon_escaped)
            bad_pattern_ids.append('U-Boot main signon')
        if self.disable_check_count['unknown_command'] == 0:
            bad_patterns.append(pattern_unknown_command)
            bad_pattern_ids.append('Unknown command')
        if self.disable_check_count['error_notification'] == 0:
            bad_patterns.append(pattern_error_notification)
            bad_pattern_ids.append('Error notification')
        try:
            self.at_prompt = False
            if send_nl:
                cmd += '\n'
            while cmd:
                # Limit max outstanding data, so UART FIFOs don't overflow
                chunk = cmd[:self.max_fifo_fill]
                cmd = cmd[self.max_fifo_fill:]
                self.p.send(chunk)
                if not wait_for_echo:
                    continue
                chunk = re.escape(chunk)
                chunk = chunk.replace('\\\n', '[\r\n]')
                m = self.p.expect([chunk] + bad_patterns)
                if m != 0:
                    self.at_prompt = False
                    raise Exception('Bad pattern found on console: ' +
                                    bad_pattern_ids[m - 1])
            if not wait_for_prompt:
                return
            m = self.p.expect([self.prompt_escaped] + bad_patterns)
            if m != 0:
                self.at_prompt = False
                raise Exception('Bad pattern found on console: ' +
                                bad_pattern_ids[m - 1])
            self.at_prompt = True
            self.at_prompt_logevt = self.logstream.logfile.cur_evt
            # Only strip \r\n; space/TAB might be significant if testing
            # indentation.
            return self.p.before.strip('\r\n')
        except Exception as ex:
            self.log.error(str(ex))
            self.cleanup_spawn()
            raise

    def ctrlc(self):
        '''Send a CTRL-C character to U-Boot.

        This is useful in order to stop execution of long-running synchronous
        commands such as "ums".

        Args:
            None.

        Returns:
            Nothing.
        '''

        self.run_command(chr(3), wait_for_echo=False, send_nl=False)

    def ensure_spawned(self):
        '''Ensure a connection to a correctly running U-Boot instance.

        This may require spawning a new Sandbox process or resetting target
        hardware, as defined by the implementation sub-class.

        This is an internal function and should not be called directly.

        Args:
            None.

        Returns:
            Nothing.
        '''

        if self.p:
            return
        try:
            self.at_prompt = False
            self.log.action('Starting U-Boot')
            self.p = self.get_spawn()
            # Real targets can take a long time to scroll large amounts of
            # text if LCD is enabled. This value may need tweaking in the
            # future, possibly per-test to be optimal. This works for 'help'
            # on board 'seaboard'.
            self.p.timeout = 30000
            self.p.logfile_read = self.logstream
            if self.config.buildconfig.get('CONFIG_SPL', False) == 'y':
                self.p.expect([pattern_u_boot_spl_signon])
                self.u_boot_spl_signon = self.p.after
                self.u_boot_spl_signon_escaped = re.escape(self.p.after)
            else:
                self.u_boot_spl_signon = None
            self.p.expect([pattern_u_boot_main_signon])
            self.u_boot_main_signon = self.p.after
            self.u_boot_main_signon_escaped = re.escape(self.p.after)
            build_idx = self.u_boot_main_signon.find(', Build:')
            if build_idx == -1:
                self.u_boot_version_string = self.u_boot_main_signon
            else:
                self.u_boot_version_string = self.u_boot_main_signon[:build_idx]
            while True:
                match = self.p.expect([self.prompt_escaped,
                                       pattern_stop_autoboot_prompt])
                if match == 1:
                    self.p.send(chr(3)) # CTRL-C
                    continue
                break
            self.at_prompt = True
            self.at_prompt_logevt = self.logstream.logfile.cur_evt
        except Exception as ex:
            self.log.error(str(ex))
            self.cleanup_spawn()
            raise

    def cleanup_spawn(self):
        '''Shut down all interaction with the U-Boot instance.

        This is used when an error is detected prior to re-establishing a
        connection with a fresh U-Boot instance.

        This is an internal function and should not be called directly.

        Args:
            None.

        Returns:
            Nothing.
        '''

        try:
            if self.p:
                self.p.close()
        except:
            pass
        self.p = None

    def validate_version_string_in_text(self, text):
        '''Assert that a command's output includes the U-Boot signon message.

        This is primarily useful for validating the "version" command without
        duplicating the signon text regex in a test function.

        Args:
            text: The command output text to check.

        Returns:
            Nothing. An exception is raised if the validation fails.
        '''

        assert(self.u_boot_version_string in text)

    def disable_check(self, check_type):
        '''Temporarily disable an error check of U-Boot's output.

        Create a new context manager (for use with the "with" statement) which
        temporarily disables a particular console output error check.

        Args:
            check_type: The type of error-check to disable. Valid values may
            be found in self.disable_check_count above.

        Returns:
            A context manager object.
        '''

        return ConsoleDisableCheck(self, check_type)

    def find_ram_base(self):
        '''Find the running U-Boot's RAM location.

        Probe the running U-Boot to determine the address of the first bank
        of RAM. This is useful for tests that test reading/writing RAM, or
        load/save files that aren't associated with some standard address
        typically represented in an environment variable such as
        ${kernel_addr_r}. The value is cached so that it only needs to be
        actively read once.

        Args:
            None.

        Returns:
            The address of U-Boot's first RAM bank, as an integer.
        '''

        if self.config.buildconfig.get('config_cmd_bdi', 'n') != 'y':
            pytest.skip('bdinfo command not supported')
        if self.ram_base == -1:
            pytest.skip('Previously failed to find RAM bank start')
        if self.ram_base is not None:
            return self.ram_base

        with self.log.section('find_ram_base'):
            response = self.run_command('bdinfo')
            for l in response.split('\n'):
                if '-> start' in l:
                    self.ram_base = int(l.split('=')[1].strip(), 16)
                    break
            if self.ram_base is None:
                self.ram_base = -1
                raise Exception('Failed to find RAM bank start in `bdinfo`')

        return self.ram_base
