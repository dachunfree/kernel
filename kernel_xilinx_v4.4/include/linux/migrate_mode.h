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
	MIGRATE_ASYNC,    //异步方式。不允许阻塞
	MIGRATE_SYNC_LIGHT, //轻量级同步，允许大多数阻塞，但是不允许把脏页写会到存储设备(费时)
	MIGRATE_SYNC,  //同步方式，允许阻塞，允许把脏页写回到存储设备上。
};

#endif		/* MIGRATE_MODE_H_INCLUDED */
