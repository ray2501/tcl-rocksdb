/*
 * For C++ compilers, use extern "C"
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <rocksdb/db.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <tcl.h>

/*
 * Only the _Init function is exported.
 */

extern DLLEXPORT int    Rocksdb_Init(Tcl_Interp * interp);

/*
 * end block for C++
 */

#ifdef __cplusplus
}
#endif

typedef struct ThreadSpecificData {
  int initialized;                /* initialization flag */
  Tcl_HashTable *rocksdb_hashtblPtr; /* per thread hash table. */
  int dbi_count;
  int itr_count;
} ThreadSpecificData;

static Tcl_ThreadDataKey dataKey;

TCL_DECLARE_MUTEX(myMutex);


void ROCKSDB_Thread_Exit(ClientData clientdata)
{
  ThreadSpecificData *tsdPtr = (ThreadSpecificData *)
      Tcl_GetThreadData(&dataKey, sizeof(ThreadSpecificData));

  if(tsdPtr->rocksdb_hashtblPtr) {
    Tcl_DeleteHashTable(tsdPtr->rocksdb_hashtblPtr);
    ckfree(tsdPtr->rocksdb_hashtblPtr);
  }
}


static int ROCKSDB_ITR(void *cd, Tcl_Interp *interp, int objc,Tcl_Obj *const*objv){
  int choice;
  rocksdb::Iterator* it;
  Tcl_HashEntry *hashEntryPtr;
  char *itrHandle;

  ThreadSpecificData *tsdPtr = (ThreadSpecificData *)
      Tcl_GetThreadData(&dataKey, sizeof(ThreadSpecificData));

  if (tsdPtr->initialized == 0) {
    tsdPtr->initialized = 1;
    tsdPtr->rocksdb_hashtblPtr = (Tcl_HashTable *) ckalloc(sizeof(Tcl_HashTable));
    Tcl_InitHashTable(tsdPtr->rocksdb_hashtblPtr, TCL_STRING_KEYS);
  }
  static const char *ITR_strs[] = {
    "seektofirst",
    "seektolast",
    "seek",
    "valid",
    "next",
    "prev",
    "key",
    "value",
    "close",
    0
  };

  enum ITR_enum {
    ITR_SEEKTOFIRST,
    ITR_SEEKTOLAST,
    ITR_SEEK,
    ITR_VALID,
    ITR_NEXT,
    ITR_PREV,
    ITR_KEY,
    ITR_VALUE,
    ITR_CLOSE,
  };

  if( objc < 2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "SUBCOMMAND ...");
    return TCL_ERROR;
  }

  if( Tcl_GetIndexFromObj(interp, objv[1], ITR_strs, "option", 0, &choice) ){
    return TCL_ERROR;
  }

  /*
   * Get the rocksdb::Iterator value
   */
  itrHandle = Tcl_GetStringFromObj(objv[0], 0);
  hashEntryPtr = Tcl_FindHashEntry( tsdPtr->rocksdb_hashtblPtr, itrHandle );
  if( !hashEntryPtr ) {
    if( interp ) {
        Tcl_Obj *resultObj = Tcl_GetObjResult( interp );

        Tcl_AppendStringsToObj( resultObj, "invalid iterator handle ", itrHandle, (char *)NULL );
    }

    return TCL_ERROR;
  }

  it = (rocksdb::Iterator *)(uintptr_t)Tcl_GetHashValue( hashEntryPtr );

  switch( (enum ITR_enum)choice ){

    case ITR_SEEKTOFIRST: {
      if( objc != 2 ){
        Tcl_WrongNumArgs(interp, 2, objv, 0);
        return TCL_ERROR;
      }

      it->SeekToFirst();
      if(!it->status().ok()) {
        Tcl_AppendResult(interp, "Error: seektofirst failed", (char*)0);
        return TCL_ERROR;
      }

      Tcl_SetObjResult(interp, Tcl_NewIntObj( 0 ));

      break;
    }

    case ITR_SEEKTOLAST: {
      if( objc != 2 ){
        Tcl_WrongNumArgs(interp, 2, objv, 0);
        return TCL_ERROR;
      }

      it->SeekToLast();
      if(!it->status().ok()) {
        Tcl_AppendResult(interp, "Error: seektolast failed", (char*)0);
        return TCL_ERROR;
      }

      Tcl_SetObjResult(interp, Tcl_NewIntObj( 0 ));

      break;
    }

    case ITR_SEEK: {
      const char *key = NULL;
      int len = 0;
      rocksdb::Slice slice;

      if( objc != 3 ){
        Tcl_WrongNumArgs(interp, 2, objv, "key ");
        return TCL_ERROR;
      }

      key = Tcl_GetStringFromObj(objv[2], &len);
      if(!key || len < 0) {
        Tcl_AppendResult(interp, "Error: key is empty", (char*)0);
        return TCL_ERROR;
      }

      slice = rocksdb::Slice(key, len);
      it->Seek(slice);
      if(!it->status().ok()) {
        Tcl_AppendResult(interp, "Error: seek failed", (char*)0);
        return TCL_ERROR;
      }

      Tcl_SetObjResult(interp, Tcl_NewIntObj( 0 ));

      break;
    }

    case ITR_VALID: {
      bool isValid;

      if( objc != 2 ){
        Tcl_WrongNumArgs(interp, 2, objv, 0);
        return TCL_ERROR;
      }

      isValid = it->Valid();
      if(!it->status().ok()) {
        Tcl_AppendResult(interp, "Error: valid failed", (char*)0);
        return TCL_ERROR;
      }

      if( isValid ) {
          Tcl_SetObjResult(interp, Tcl_NewBooleanObj( 1 ));
      } else {
          Tcl_SetObjResult(interp, Tcl_NewBooleanObj( 0 ));
      }

      break;
    }

    case ITR_NEXT: {
      if( objc != 2 ){
        Tcl_WrongNumArgs(interp, 2, objv, 0);
        return TCL_ERROR;
      }

      it->Next();
      if(!it->status().ok()) {
        Tcl_AppendResult(interp, "Error: next failed", (char*)0);
        return TCL_ERROR;
      }

      Tcl_SetObjResult(interp, Tcl_NewIntObj( 0 ));

      break;
    }

    case ITR_PREV: {
      if( objc != 2 ){
        Tcl_WrongNumArgs(interp, 2, objv, 0);
        return TCL_ERROR;
      }

      it->Prev();
      if(!it->status().ok()) {
        Tcl_AppendResult(interp, "Error: prev failed", (char*)0);
        return TCL_ERROR;
      }

      Tcl_SetObjResult(interp, Tcl_NewIntObj( 0 ));

      break;
    }

    case ITR_KEY: {
      std::string key2;
      Tcl_Obj *pResultStr = NULL;

      if( objc != 2 ){
        Tcl_WrongNumArgs(interp, 2, objv, 0);
        return TCL_ERROR;
      }

      key2 = it->key().ToString();
      if(!it->status().ok()) {
        Tcl_AppendResult(interp, "Error: key failed", (char*)0);
        return TCL_ERROR;
      }

      pResultStr = Tcl_NewStringObj(key2.c_str(), key2.length());
      Tcl_SetObjResult(interp, pResultStr);

      break;
    }

    case ITR_VALUE: {
      std::string value2;
      Tcl_Obj *pResultStr = NULL;

      if( objc != 2 ){
        Tcl_WrongNumArgs(interp, 2, objv, 0);
        return TCL_ERROR;
      }

      value2 = it->value().ToString();
      if(!it->status().ok()) {
        Tcl_AppendResult(interp, "Error: value failed", (char*)0);
        return TCL_ERROR;
      }

      pResultStr = Tcl_NewStringObj(value2.c_str(), value2.length());
      Tcl_SetObjResult(interp, pResultStr);

      break;
    }

    case ITR_CLOSE: {
      if( objc != 2 ){
        Tcl_WrongNumArgs(interp, 2, objv, 0);
        return TCL_ERROR;
      }

      delete it;

      Tcl_MutexLock(&myMutex);
      if( hashEntryPtr )  Tcl_DeleteHashEntry(hashEntryPtr);
      Tcl_MutexUnlock(&myMutex);

      Tcl_DeleteCommand(interp, itrHandle);
      Tcl_SetObjResult(interp, Tcl_NewIntObj( 0 ));

      break;
    }
  }

  return TCL_OK;
}

static int ROCKSDB_DBI(void *cd, Tcl_Interp *interp, int objc,Tcl_Obj *const*objv){
  int choice;
  rocksdb::DB* db;
  Tcl_HashEntry *hashEntryPtr;
  char *dbiHandle;

  ThreadSpecificData *tsdPtr = (ThreadSpecificData *)
      Tcl_GetThreadData(&dataKey, sizeof(ThreadSpecificData));

  if (tsdPtr->initialized == 0) {
    tsdPtr->initialized = 1;
    tsdPtr->rocksdb_hashtblPtr = (Tcl_HashTable *) ckalloc(sizeof(Tcl_HashTable));
    Tcl_InitHashTable(tsdPtr->rocksdb_hashtblPtr, TCL_STRING_KEYS);
  }

  static const char *DBI_strs[] = {
    "get",
    "put",
    "delete",
    "exists",
    "iterator",
    "close",
    0
  };

  enum DBI_enum {
    DBI_GET,
    DBI_PUT,
    DBI_DELETE,
    DBI_EXISTS,
    DBI_ITERATOR,
    DBI_CLOSE,
  };

  if( objc < 2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "SUBCOMMAND ...");
    return TCL_ERROR;
  }

  if( Tcl_GetIndexFromObj(interp, objv[1], DBI_strs, "option", 0, &choice) ){
    return TCL_ERROR;
  }

  /*
   * Get the rocksdb::DB value
   */
  dbiHandle = Tcl_GetStringFromObj(objv[0], 0);
  hashEntryPtr = Tcl_FindHashEntry( tsdPtr->rocksdb_hashtblPtr, dbiHandle );
  if( !hashEntryPtr ) {
    if( interp ) {
        Tcl_Obj *resultObj = Tcl_GetObjResult( interp );

        Tcl_AppendStringsToObj( resultObj, "invalid db handle ", dbiHandle, (char *)NULL );
    }

    return TCL_ERROR;
  }

  db = (rocksdb::DB *)(uintptr_t)Tcl_GetHashValue( hashEntryPtr );

  switch( (enum DBI_enum)choice ){

    case DBI_GET: {
      rocksdb::Status status;
      char *key;
      int key_len;
      std::string key2;
      std::string value2;
      Tcl_Obj *pResultStr = NULL;

      if( objc < 3 || (objc&1)!=1) {
        Tcl_WrongNumArgs(interp, 2, objv, "key ");
        return TCL_ERROR;
      }

      key = Tcl_GetStringFromObj(objv[2], &key_len);
      if( !key || key_len < 1 ){
         Tcl_AppendResult(interp, "Error: key is an empty key ", (char*)0);
         return TCL_ERROR;
      }

      key2 = key;

      status = db->Get(rocksdb::ReadOptions(), key2, &value2);
      if(!status.ok()) {
        Tcl_AppendResult(interp, "Error: get failed", (char*)0);
        return TCL_ERROR;
      }

      pResultStr = Tcl_NewStringObj(value2.c_str(), value2.length());
      Tcl_SetObjResult(interp, pResultStr);

      break;
    }

    case DBI_PUT: {
      rocksdb::Status status;
      rocksdb::WriteOptions write_options;
      char *key;
      char *data;
      int key_len;
      int data_len;
      std::string key2;
      std::string value2;
      char *zArg;
      int i = 0;

      if( objc < 4 || (objc&1)!=0) {
        Tcl_WrongNumArgs(interp, 2, objv, "key data ?-sync BOOLEAN? ");
        return TCL_ERROR;
      }

      key = Tcl_GetStringFromObj(objv[2], &key_len);
      if( !key || key_len < 1 ){
         Tcl_AppendResult(interp, "Error: key is an empty key ", (char*)0);
         return TCL_ERROR;
      }

      data = Tcl_GetStringFromObj(objv[3], &data_len);
      if( !data || data_len < 1 ){
         Tcl_AppendResult(interp, "Error: data is an empty value ", (char*)0);
         return TCL_ERROR;
      }

      for(i=4; i+1<objc; i+=2){
        zArg = Tcl_GetStringFromObj(objv[i], 0);

        if( strcmp(zArg, "-sync")==0 ){
            int b;
            if( Tcl_GetBooleanFromObj(interp, objv[i+1], &b) ) return TCL_ERROR;
            if( b ){
              write_options.sync = true;
            }else{
              write_options.sync = false;
            }
        } else{
           Tcl_AppendResult(interp, "unknown option: ", zArg, (char*)0);
           return TCL_ERROR;
        }
      }


      key2 = key;
      value2 = data;
      status = db->Put(write_options, key2, value2);
      if(!status.ok()) {
        Tcl_AppendResult(interp, "Error: put failed", (char*)0);
        return TCL_ERROR;
      }

      Tcl_SetObjResult(interp, Tcl_NewIntObj( 0 ));

      break;
    }

    case DBI_DELETE: {
      rocksdb::Status status;
      char *key;
      int key_len;
      std::string key2;

      if( objc < 3 || (objc&1)!=1) {
        Tcl_WrongNumArgs(interp, 2, objv, "key ");
        return TCL_ERROR;
      }

      key = Tcl_GetStringFromObj(objv[2], &key_len);
      if( !key || key_len < 1 ){
         Tcl_AppendResult(interp, "Error: key is an empty key ", (char*)0);
         return TCL_ERROR;
      }

      key2 = key;

      status = db->Delete(rocksdb::WriteOptions(), key2);
      if(!status.ok()) {
        Tcl_AppendResult(interp, "Error: delete failed", (char*)0);
        return TCL_ERROR;
      }

      Tcl_SetObjResult(interp, Tcl_NewIntObj( 0 ));

      break;
    }

    case DBI_EXISTS: {
      char *key;
      int key_len;
      std::string key2;
      std::string value2;
      bool result;

      if( objc < 3 || (objc&1)!=1) {
        Tcl_WrongNumArgs(interp, 2, objv, "key ");
        return TCL_ERROR;
      }

      key = Tcl_GetStringFromObj(objv[2], &key_len);
      if( !key || key_len < 1 ){
         Tcl_AppendResult(interp, "Error: key is an empty key ", (char*)0);
         return TCL_ERROR;
      }

      key2 = key;

      result = db->KeyMayExist(rocksdb::ReadOptions(), key2, &value2);

      if(result) {
          Tcl_SetObjResult(interp, Tcl_NewBooleanObj( 1 ));
      } else {
          Tcl_SetObjResult(interp, Tcl_NewBooleanObj( 0 ));
      }

      break;
    }

    case DBI_ITERATOR: {
      Tcl_HashEntry *newHashEntryPtr;
      char handleName[16 + TCL_INTEGER_SPACE];
      Tcl_Obj *pResultStr = NULL;
      int newvalue;

      if( objc != 2 ) {
        Tcl_WrongNumArgs(interp, 2, objv, 0);
        return TCL_ERROR;
      }

      rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());

      Tcl_MutexLock(&myMutex);
      sprintf( handleName, "rocksitr%d", tsdPtr->itr_count++ );

      pResultStr = Tcl_NewStringObj( handleName, -1 );

      newHashEntryPtr = Tcl_CreateHashEntry(tsdPtr->rocksdb_hashtblPtr, handleName, &newvalue);
      Tcl_SetHashValue(newHashEntryPtr, (ClientData)(uintptr_t) it);
      Tcl_MutexUnlock(&myMutex);

      Tcl_CreateObjCommand(interp, handleName, (Tcl_ObjCmdProc *) ROCKSDB_ITR,
          (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);

      Tcl_SetObjResult(interp, pResultStr);

      break;
    }

    case DBI_CLOSE: {
      if( objc != 2 ){
        Tcl_WrongNumArgs(interp, 2, objv, 0);
        return TCL_ERROR;
      }

      delete db;

      Tcl_MutexLock(&myMutex);
      if( hashEntryPtr )  Tcl_DeleteHashEntry(hashEntryPtr);
      Tcl_MutexUnlock(&myMutex);

      Tcl_DeleteCommand(interp, dbiHandle);
      Tcl_SetObjResult(interp, Tcl_NewIntObj( 0 ));

      break;
    }
  }

  return TCL_OK;
}


static int ROCKSDB_MAIN(void *cd, Tcl_Interp *interp, int objc,Tcl_Obj *const*objv){
  int choice;

  ThreadSpecificData *tsdPtr = (ThreadSpecificData *)
      Tcl_GetThreadData(&dataKey, sizeof(ThreadSpecificData));

  if (tsdPtr->initialized == 0) {
    tsdPtr->initialized = 1;
    tsdPtr->rocksdb_hashtblPtr = (Tcl_HashTable *) ckalloc(sizeof(Tcl_HashTable));
    Tcl_InitHashTable(tsdPtr->rocksdb_hashtblPtr, TCL_STRING_KEYS);
  }

  static const char *DB_strs[] = {
    "open",
    "version",
    0
  };

  enum DB_enum {
    DB_OPEN,
    DB_VERSION,
  };

  if( objc < 2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "SUBCOMMAND ...");
    return TCL_ERROR;
  }

  if( Tcl_GetIndexFromObj(interp, objv[1], DB_strs, "option", 0, &choice) ){
    return TCL_ERROR;
  }

  switch( (enum DB_enum)choice ){

    case DB_OPEN: {
      char *zArg;
      rocksdb::DB* db;
      rocksdb::Options options;
      rocksdb::Status status;
      Tcl_HashEntry *newHashEntryPtr;
      const char *path = NULL;
      int len;
      char handleName[16 + TCL_INTEGER_SPACE];
      Tcl_Obj *pResultStr = NULL;
      int newvalue;
      int i = 0;
      bool readonly = false;

      // Optimize RocksDB.
      options.IncreaseParallelism();
      options.OptimizeLevelStyleCompaction();

      if( objc < 4 || (objc&1)!=0 ){
          Tcl_WrongNumArgs(interp, 2, objv,
          "-path path ?-readonly BOOLEAN? ?-create_if_missing BOOLEAN? \
           ?-error_if_exists BOOLEAN? ?-paranoid_checks BOOLEAN? \
           ?-write_buffer_size size? ?-max_write_buffer_number number? \
           ?-target_file_size_base size? ?-max_open_files number? "
          );

        return TCL_ERROR;
      }

      for(i=2; i+1<objc; i+=2){
        zArg = Tcl_GetStringFromObj(objv[i], 0);

        if( strcmp(zArg, "-path")==0 ){
            path = Tcl_GetStringFromObj(objv[i+1], &len);
            if(!path || len < 0) {
                return TCL_ERROR;
            }
        } else if( strcmp(zArg, "-readonly")==0 ){
            int b;
            if( Tcl_GetBooleanFromObj(interp, objv[i+1], &b) ) return TCL_ERROR;
            if( b ){
              readonly = true;
            }else{
              readonly = false;
            }
        } else if( strcmp(zArg, "-create_if_missing")==0 ){
            int b;
            if( Tcl_GetBooleanFromObj(interp, objv[i+1], &b) ) return TCL_ERROR;
            if( b ){
              options.create_if_missing = true;
            }else{
              options.create_if_missing = false;
            }
        } else if( strcmp(zArg, "-error_if_exists")==0 ){
            int b;
            if( Tcl_GetBooleanFromObj(interp, objv[i+1], &b) ) return TCL_ERROR;
            if( b ){
              options.error_if_exists = true;
            }else{
              options.error_if_exists = false;
            }
        } else if( strcmp(zArg, "-paranoid_checks")==0 ){
            int b;
            if( Tcl_GetBooleanFromObj(interp, objv[i+1], &b) ) return TCL_ERROR;
            if( b ){
              options.paranoid_checks = true;
            }else{
              options.paranoid_checks = false;
            }
        } else if( strcmp(zArg, "-write_buffer_size")==0 ){
            int size = 0;

            if(Tcl_GetIntFromObj(interp, objv[i+1], &size) != TCL_OK) {
                return TCL_ERROR;
            }

            if( size > 0 ) {
              options.write_buffer_size = size;
            }
        } else if( strcmp(zArg, "-max_write_buffer_number")==0 ){
            int number = 0;

            if(Tcl_GetIntFromObj(interp, objv[i+1], &number) != TCL_OK) {
                return TCL_ERROR;
            }

            if( number > 0 ) {
              options.max_write_buffer_number = number;
            }
        } else if( strcmp(zArg, "-target_file_size_base")==0 ){
            int size = 0;

            if(Tcl_GetIntFromObj(interp, objv[i+1], &size) != TCL_OK) {
                return TCL_ERROR;
            }

            if( size > 0 ) {
              options.target_file_size_base = size;
            }
        } else if( strcmp(zArg, "-max_open_files")==0 ){
            int number = 0;

            if(Tcl_GetIntFromObj(interp, objv[i+1], &number) != TCL_OK) {
                return TCL_ERROR;
            }

            if(number < 0) {
                options.max_open_files = -1;
            } else {
                options.max_open_files = number;
            }
        } else{
           Tcl_AppendResult(interp, "unknown option: ", zArg, (char*)0);
           return TCL_ERROR;
        }
      }

      if(!path) {
          if( interp ) {
            Tcl_Obj *resultObj = Tcl_GetObjResult( interp );
            Tcl_AppendStringsToObj( resultObj, "No database path", (char *)NULL );
          }

          return TCL_ERROR;
      }

      if(readonly == false) {
          status = rocksdb::DB::Open(options, path, &db);
      } else {
          status = rocksdb::DB::OpenForReadOnly(options, path, &db);
      }

      if(!status.ok()) {
          if( interp ) {
            Tcl_Obj *resultObj = Tcl_GetObjResult( interp );
            Tcl_AppendStringsToObj( resultObj, "ERROR: open failed", (char *)NULL );
          }

          return TCL_ERROR;
      }

      Tcl_MutexLock(&myMutex);
      sprintf( handleName, "rocksdbi%d", tsdPtr->dbi_count++ );

      pResultStr = Tcl_NewStringObj( handleName, -1 );

      newHashEntryPtr = Tcl_CreateHashEntry(tsdPtr->rocksdb_hashtblPtr, handleName, &newvalue);
      Tcl_SetHashValue(newHashEntryPtr, (ClientData)(uintptr_t) db);
      Tcl_MutexUnlock(&myMutex);


      Tcl_CreateObjCommand(interp, handleName, (Tcl_ObjCmdProc *) ROCKSDB_DBI,
          (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);

      Tcl_SetObjResult(interp, pResultStr);

      break;
    }

    case DB_VERSION: {
      int major = 0, minor = 0, patch = 0;
      Tcl_Obj *pResultStr = NULL;

      if( objc != 2){
        Tcl_WrongNumArgs(interp, 2, objv, 0);
        return TCL_ERROR;
      }

      major = ROCKSDB_MAJOR;
      minor = ROCKSDB_MINOR;
      patch = ROCKSDB_PATCH;

      pResultStr = Tcl_NewListObj(0, NULL);
      Tcl_ListObjAppendElement(interp, pResultStr, Tcl_NewIntObj(major));
      Tcl_ListObjAppendElement(interp, pResultStr, Tcl_NewIntObj(minor));
      Tcl_ListObjAppendElement(interp, pResultStr, Tcl_NewIntObj(patch));

      Tcl_SetObjResult(interp,  pResultStr);

      break;
    }
  }

  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Rocksdb_Init --
 *
 *	Initialize the new package.
 *
 * Results:
 *	A standard Tcl result
 *
 * Side effects:
 *	The Rocksdb package is created.
 *
 *----------------------------------------------------------------------
 */

int
Rocksdb_Init(Tcl_Interp *interp)
{
    if (Tcl_InitStubs(interp, "8.5", 0) == NULL) {
	return TCL_ERROR;
    }

    if (Tcl_PkgProvide(interp, PACKAGE_NAME, PACKAGE_VERSION) != TCL_OK) {
	return TCL_ERROR;
    }

    /*
     *   Tcl_GetThreadData handles the auto-initialization of all data in
     *  the ThreadSpecificData to NULL at first time.
     */
    Tcl_MutexLock(&myMutex);
    ThreadSpecificData *tsdPtr = (ThreadSpecificData *)
        Tcl_GetThreadData(&dataKey, sizeof(ThreadSpecificData));

    if (tsdPtr->initialized == 0) {
        tsdPtr->initialized = 1;
        tsdPtr->rocksdb_hashtblPtr = (Tcl_HashTable *) ckalloc(sizeof(Tcl_HashTable));
        Tcl_InitHashTable(tsdPtr->rocksdb_hashtblPtr, TCL_STRING_KEYS);

        tsdPtr->dbi_count = 0;
        tsdPtr->itr_count = 0;
    }
    Tcl_MutexUnlock(&myMutex);

    /* Add a thread exit handler to delete hash table */
    Tcl_CreateThreadExitHandler(ROCKSDB_Thread_Exit, (ClientData)NULL);

    Tcl_CreateObjCommand(interp, "rocksdb", (Tcl_ObjCmdProc *) ROCKSDB_MAIN,
        (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);

    return TCL_OK;
}
