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
	MIGRATE_ASYNC,    //�첽��ʽ������������
	MIGRATE_SYNC_LIGHT, //������ͬ���������������������ǲ��������ҳд�ᵽ�洢�豸(��ʱ)
	MIGRATE_SYNC,  //ͬ����ʽ�������������������ҳд�ص��洢�豸�ϡ�
};

#endif		/* MIGRATE_MODE_H_INCLUDED */
