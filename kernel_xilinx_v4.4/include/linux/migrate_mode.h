#ifndef MIGRATE_MODE_H_INCLUDED
#define MIGRATE_MODE_H_INCLUDED
/*
 * MIGRATE_ASYNC means never block
 * MIGRATE_SYNC_LIGHT in the current implementation means to allow blocking
 *	on most operations but not ->writepage as the potential stall time
 *	is too significant
 * MIGRATE_SYNC will block when migrating pages
 */

enum migrate_mode {
    /*
	 *�첽ģʽ��
	 *�ڴ���Ƭ������õ�ģʽ��Ĭ�ϳ�ʼ���첽ģʽ�����ڴ�ģʽ�в����������(����ʱ��Ƭ���˿��Խ���������
	 *��)��Ҳ���Ǵ���ģʽ������ļ�ҳ���д����ļ�ҳ����ӳ���ļ�����ʹ�ã�����ģʽҲ�Ƕ�����ϵͳѹ����С
	 *��ģʽ�������ƶ���ҳ����MIGRATE_MOVABLE��MIGRATE_CMA���͵�ҳ�򣩡�
     */
	MIGRATE_ASYNC,
    /*
     *��ͬ��ģʽ��
     *���첽ģʽ�����˸����ڴ�ʱ������������»�ʹ����ͬ��ģʽ�ٴ������ڴ棺1.��ȷ��ʾ����Ĳ���͸����
     *ҳ������£�2.��ǰ�������ں��̵߳�����¡����ģʽ����������������������(���������̫��ҳ����Ҫ��
     *���ȴ�һ��ʱ�䣬�豸��æ����ȴ�һС�ᣬ����æ��������ֱ���õ���Ϊֹ)������ģʽ�ᴦ������ҳ���ļ�
     *ҳ�����ǲ�������ļ�ҳִ�л�д���������������ҳ���ڻ�дʱ��Ҳ����ȴ����д������
     */
	MIGRATE_SYNC_LIGHT,
    /*
     *ͬ��ģʽ:
     *���в��������Խ������������һ�ȴ������ҳ��д������������ļ�ҳ������ҳ���л�д�����̣����Ե�����
     *�ķ�ϵͳ��Դ����ϵͳ��ɵ�ѹ�������������������·�����
     *	1.��cma�з����ڴ�ʱ��
     *	2.����alloc_contig_range()���Է���һ��ָ���˿�ʼҳ��źͽ���ҳ��ŵ�����ҳ��ʱ��
     *	3.ͨ��д��1��sysfs�е�/vm/compact_memory�ļ��ֶ�ʵ��ͬ���ڴ���Ƭ����
     *ͬ��ģʽ�������Ƴټ�������ֵ��������ͬ��ģʽ�£������ú�compact_control����ͬ��ģʽʱ����
     *pageblock��PB_migrate_skip���
     */
	MIGRATE_SYNC,
};
#endif		/* MIGRATE_MODE_H_INCLUDED */
