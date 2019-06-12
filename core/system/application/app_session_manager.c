/*©mit**************************************************************************
*                                                                              *
* This file is part of FRIEND UNIFYING PLATFORM.                               *
* Copyright (c) Friend Software Labs AS. All rights reserved.                  *
*                                                                              *
* Licensed under the Source EULA. Please refer to the copy of the MIT License, *
* found in the file license_mit.txt.                                           *
*                                                                              *
*****************************************************************************©*/
/** @file
 *
 *  App Session Manager
 *
 * file contain all functitons related to application sessions management
 *
 *  @author PS (Pawel Stefanski)
 *  @date created 11/2016
 */
#include "app_session_manager.h"
#include <system/systembase.h>
#include "app_session.h"
#include <core/functions.h>

/**
 * Create Application Session Manager
 *
 * @return AppSessionManager
 */

AppSessionManager *AppSessionManagerNew()
{
	AppSessionManager *as = NULL;
	
	if( ( as = FCalloc( 1, sizeof( AppSessionManager ) ) ) != NULL )
	{
		pthread_mutex_init( &(as->sl_Mutex), NULL );
	}
	else
	{
		FERROR("Cannot allocate memory for AppSessionManager\n");
	}
	
	return as;
}

/**
 * Delete Application Session Manager
 *
 * @param as application session to remove
 */

void AppSessionManagerDelete( AppSessionManager *as )
{
	DEBUG("[AppSessionManagerGetSession] AppSessionManagerDelete\n");
	if( as )
	{
		AppSession *las = as->sl_AppSessions;
		if( FRIEND_MUTEX_LOCK( &(as->sl_Mutex) ) == 0 )
		{
			AppSession *oas = las;
		
			while( las != NULL )
			{
				DEBUG("[AppSessionManagerGetSession] AppSession will be removed from list\n");

				oas = las;
				las =(AppSession  *)las->node.mln_Succ;
				AppSessionDelete( oas );
			}
		
			FRIEND_MUTEX_UNLOCK( &(as->sl_Mutex) );
		}
		pthread_mutex_destroy( &(as->sl_Mutex) );
		
		FFree( as );
	}
}

/**
 * Add application session to manager
 *
 * @param as application session
 * @param nas pointer to application session which will be added
 * @return 0 if success, otherwise error number
 */

int AppSessionManagerAddSession( AppSessionManager *as, AppSession *nas )
{
	if( as != NULL )
	{
		AppSession *las = NULL;
		
		if( FRIEND_MUTEX_LOCK( &(as->sl_Mutex) ) == 0 )
		{
			LIST_FOR_EACH( as->sl_AppSessions, las, AppSession * )
			{
				if( nas->as_SASID == las->as_SASID )
				{
					DEBUG("[AppSessionManagerGetSession] AppSession was already added to list\n");
					FRIEND_MUTEX_UNLOCK( &(as->sl_Mutex) );
					return 0;
				}
			}
			FRIEND_MUTEX_UNLOCK( &(as->sl_Mutex) );
		}
		
		if( FRIEND_MUTEX_LOCK( &(as->sl_Mutex) ) == 0 )
		{
			AppSession *lastone = as->sl_AppSessions;
			if( lastone == NULL )
			{
				as->sl_AppSessions = nas;
			}
			else
			{
				while( lastone->node.mln_Succ != NULL )
				{
					lastone = (AppSession *)lastone->node.mln_Succ;
				}
				lastone->node.mln_Succ = (MinNode *)nas;
			}
			FRIEND_MUTEX_UNLOCK( &(as->sl_Mutex) );
		}
		return 0;
	}
	return -2;
}

/**
 * Remove application session from manager
 *
 * @param as application session
 * @param nas pointer to application session which will be removed
 * @return 0 if success, otherwise error number
 */

int AppSessionManagerRemSession( AppSessionManager *as, AppSession *nas )
{
	if( as != NULL )
	{
		AppSession *las = NULL;
		
		if( FRIEND_MUTEX_LOCK( &(as->sl_Mutex) ) == 0 )
		{
			AppSession *oas = as->sl_AppSessions;	// old application session
		
			LIST_FOR_EACH( as->sl_AppSessions, las, AppSession * )
			{
				if( nas->as_SASID == las->as_SASID )
				{
					DEBUG("[AppSessionManagerGetSession] AppSession will be removed from list\n");
				
					if( nas == as->sl_AppSessions )	// if session is equal to first entry, we only overwrite pointer
					{
						as->sl_AppSessions = (AppSession *) nas->node.mln_Succ;
					}
					else	// if session is not first entry then we only update next pointer in previous pointer
					{
						oas->node.mln_Succ = (MinNode *)nas->node.mln_Succ;
					}
				
					AppSessionDelete( nas );
				
					DEBUG("[AppSessionManagerRemSession] appsession removed\n");
					FRIEND_MUTEX_UNLOCK( &(as->sl_Mutex) );
					return 0;
				}
				oas = las;
			}
			FRIEND_MUTEX_UNLOCK( &(as->sl_Mutex) );
		}
	}
	else
	{
		DEBUG("[AppSessionManagerRemSession] application session do not exist!\n");
		return -1;
	}
	DEBUG("[AppSessionManagerRemSession] appsession not found\n");
	return -2;
}

/**
 * Get
 *
 * @param as application session
 * @param id of application session to be searched
 * @return application session
 */

AppSession *AppSessionManagerGetSession( AppSessionManager *as, FUQUAD id )
{
	if( as != NULL )
	{
		AppSession *las = NULL;
		
		LIST_FOR_EACH( as->sl_AppSessions, las, AppSession * )
		{
			if( id == las->as_SASID )
			{
				DEBUG("[AppSessionManagerGetSession] AppSession found\n");
				
				return las;
			}
		}
	}
	else
	{
		return NULL;
	}
	return NULL;
}
