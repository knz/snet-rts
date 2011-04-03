#include <stdarg.h>
#include "interface_functions.h"
#include "distribution.h"
#include "record.h"
#include "memfun.h"
//#include "debug.h"
#include "map.h"

/* ***************************************************************************/

bool SNetRecPatternMatches(snet_variantencoding_t *pat, snet_record_t *rec)
{
  int j;

  for (j = 0; j < SNetTencGetNumFields(pat); j++) {
    if (!SNetRecHasField(rec, SNetTencGetFieldNames(pat)[j])) {
      return false;
    }
  }

  for (j = 0; j < SNetTencGetNumTags(pat); j++) {
    if (!SNetRecHasTag(rec, SNetTencGetTagNames(pat)[j])) {
      return false;
    }
  }

  for (j = 0; j < SNetTencGetNumBTags(pat); j++) {
    if (!SNetRecHasBTag(rec, SNetTencGetBTagNames(pat)[j])) {
      return false;
    }
  }

  return true;
}

snet_record_t *SNetRecCreate( snet_record_descr_t descr, ...)
{
  snet_record_t *rec;
  va_list args;

  rec = SNetMemAlloc( sizeof( snet_record_t));
  REC_DESCR( rec) = descr;

  va_start( args, descr);
  switch (descr) {
    case REC_data:
      RECPTR( rec) = SNetMemAlloc( sizeof( snet_record_types_t));
      RECORD( rec, data_rec) = SNetMemAlloc( sizeof( data_rec_t));
      DATA_REC( rec, btags) = SNetintMapCreate(0, -1);
      DATA_REC( rec, tags) = SNetintMapCreate(0, -1);
      DATA_REC( rec, fields) = SNetrefMapCreate(0, NULL);
      DATA_REC( rec, mode) = MODE_binary;
      DATA_REC( rec, interface_id) = 0;
      break;
    case REC_trigger_initialiser:
      RECPTR( rec) = SNetMemAlloc( sizeof( snet_record_types_t));
      break;
    case REC_sync:
      RECPTR( rec) = SNetMemAlloc( sizeof( snet_record_types_t));
      RECORD( rec, sync_rec) = SNetMemAlloc( sizeof( sync_rec_t));
      SYNC_REC( rec, input) = va_arg( args, snet_stream_t *);
      break;
    case REC_collect:
      RECPTR( rec) = SNetMemAlloc( sizeof( snet_record_types_t));
      RECORD( rec, coll_rec) = SNetMemAlloc( sizeof( coll_rec_t));
      COLL_REC( rec, output) = va_arg( args, snet_stream_t*);
      break;
    case REC_terminate:
      RECPTR( rec) = SNetMemAlloc( sizeof( snet_record_types_t));
      break;
    case REC_sort_end:
      RECPTR( rec) = SNetMemAlloc( sizeof( snet_record_types_t));
      RECORD( rec, sort_end_rec) = SNetMemAlloc( sizeof( sort_end_t));
      SORT_E_REC( rec, level) = va_arg( args, int);
      SORT_E_REC( rec, num) = va_arg( args, int);
      break;
    default:
      //FIXME
      //SNetUtilDebugFatal("Unknown control record destription. [%d]", descr);
      break;
  }
  va_end( args);

  return rec;
}

snet_record_t *SNetRecCopy( snet_record_t *rec)
{
  int name;
  snet_ref_t *ref;
  snet_record_t *new_rec;

  switch (REC_DESCR( rec)) {
    case REC_data:
      new_rec = SNetRecCreate( REC_data);
      DATA_REC( new_rec, fields) = SNetrefMapCreate(0, NULL);

      MAP_FOR_EACH(DATA_REC( rec, fields), name, ref)
        SNetRecSetField( new_rec, name, SNetInterfaceGet(DATA_REC( rec, interface_id))->copyfun(ref));
      END_FOR

      DATA_REC( new_rec, tags) = SNetintMapCopy( DATA_REC( rec, tags));
      DATA_REC( new_rec, btags) = SNetintMapCopy( DATA_REC( rec, btags));
      SNetRecSetInterfaceId( new_rec, SNetRecGetInterfaceId( rec));
      SNetRecSetDataMode( new_rec, SNetRecGetDataMode( rec));
      break;
    case REC_sort_end:
      new_rec = SNetRecCreate( REC_DESCR( rec),  SORT_E_REC( rec, level),
                               SORT_E_REC( rec, num));
      break;
    case REC_terminate:
      new_rec = SNetRecCreate( REC_terminate);
      break;
    default:
      //FIXME
      //SNetUtilDebugFatal("Can't copy record of type %d", REC_DESCR( rec));
      break;
  }

  return new_rec;
}

void SNetRecDestroy( snet_record_t *rec)
{
  switch (REC_DESCR( rec)) {
    case REC_data:
      SNetrefMapDestroy( DATA_REC( rec, fields));
      SNetintMapDestroy( DATA_REC( rec, tags));
      SNetintMapDestroy( DATA_REC( rec, btags));
      SNetMemFree( RECORD( rec, data_rec));
      break;
    case REC_sync:
      SNetMemFree( RECORD( rec, sync_rec));
      break;
    case REC_collect:
      SNetMemFree( RECORD( rec, coll_rec));
      break;
    case REC_sort_end:
      SNetMemFree( RECORD( rec, sort_end_rec));
      break;
    case REC_terminate:
      break;
    case REC_trigger_initialiser:
      break;
    default:
      //FIXME
      //SNetUtilDebugFatal("Unknown record description, in SNetRecDestroy");
      break;
  }
  SNetMemFree( RECPTR( rec));
  SNetMemFree( rec);
}

/*****************************************************************************/

snet_record_descr_t SNetRecGetDescriptor( snet_record_t *rec)
{
  return REC_DESCR( rec);
}

/*****************************************************************************/

snet_record_t *SNetRecSetInterfaceId( snet_record_t *rec, int id)
{
  if (REC_DESCR( rec) != REC_data) {
    //FIXME
    //SNetUtilDebugFatal("SNetRecSetInterfaceId only accepts data records (%d)",
    //                   REC_DESCR(rec));
  }

  DATA_REC( rec, interface_id) = id;
  return rec;
}

int SNetRecGetInterfaceId( snet_record_t *rec)
{
  if (REC_DESCR( rec) != REC_data) {
    //FIXME
    //SNetUtilDebugFatal("SNetRecGetInterfaceId only accepts data records (%d)",
    //                   REC_DESCR(rec));
  }

  return DATA_REC( rec, interface_id);
}

/*****************************************************************************/

snet_record_t *SNetRecSetDataMode( snet_record_t *rec, snet_record_mode_t mod)
{
  if (REC_DESCR( rec) != REC_data) {
    //FIXME
    //SNetUtilDebugFatal("SNetRecSetDataMode only accepts data records (%d)",
    //                   REC_DESCR(rec));
  }

  DATA_REC( rec, mode) = mod;
  return rec;
}

snet_record_mode_t SNetRecGetDataMode( snet_record_t *rec)
{
  if (REC_DESCR( rec) != REC_data) {
    //FIXME
    //SNetUtilDebugFatal("SNetRecGetDataMode only accepts data records (%d)",
    //                   REC_DESCR(rec));
  }

  return DATA_REC( rec, mode);
}

/*****************************************************************************/

snet_stream_t *SNetRecGetStream( snet_record_t *rec)
{
  snet_stream_t *result;

  switch( REC_DESCR( rec)) {
  case REC_sync:
    result = SYNC_REC( rec, input);
    break;
  case REC_collect:
    result = COLL_REC( rec, output);
    break;
  default:
    //FIXME
    //SNetUtilDebugFatal("Wrong type in SNetRecGetStream() (%d)", REC_DESCR(rec));
    break;
  }

  return result;
}

/*****************************************************************************/

void SNetRecSetNum( snet_record_t *rec, int value)
{
  if (REC_DESCR( rec) != REC_sort_end) {
    //FIXME
    //SNetUtilDebugFatal("Wrong type in SNetRecSetNum() (%d)", REC_DESCR( rec));
  }

  SORT_E_REC( rec, num) = value;
}

int SNetRecGetNum( snet_record_t *rec)
{
  if (REC_DESCR( rec) != REC_sort_end) {
    //FIXME
    ///SNetUtilDebugFatal("Wrong type in SNetRecGetNum() (%d)", REC_DESCR( rec));
  }

  return SORT_E_REC( rec, num);
}

void SNetRecSetLevel( snet_record_t *rec, int value)
{
  if (REC_DESCR( rec) != REC_sort_end) {
    //FIXME
    ///SNetUtilDebugFatal("Wrong type in SNetRecSetLevel() (%d)", REC_DESCR( rec));
  }

  SORT_E_REC( rec, level) = value;
}

int SNetRecGetLevel( snet_record_t *rec)
{
  if (REC_DESCR( rec) != REC_sort_end) {
    //FIXME
    //SNetUtilDebugFatal("Wrong type in SNetRecSetLevel() (%d)", REC_DESCR( rec));
  }

  return SORT_E_REC( rec, level);
}

/*****************************************************************************/

void SNetRecSetTag( snet_record_t *rec, int name, int val)
{
  SNetintMapSet(DATA_REC(rec, tags), name, val);
}

int SNetRecGetTag( snet_record_t *rec, int name)
{
  return SNetintMapGet(DATA_REC(rec, tags), name);
}

int SNetRecTakeTag( snet_record_t *rec, int name)
{
  return SNetintMapTake(DATA_REC(rec, tags), name);
}

bool SNetRecHasTag( snet_record_t *rec, int name)
{
  return SNetRecGetTag(rec, name) != -1;
}

/*****************************************************************************/

void SNetRecSetBTag( snet_record_t *rec, int name, int val)
{
  SNetintMapSet(DATA_REC(rec, btags), name, val);
}

int SNetRecGetBTag( snet_record_t *rec, int name)
{
  return SNetintMapGet(DATA_REC(rec, btags), name);
}

int SNetRecTakeBTag( snet_record_t *rec, int name)
{
  return SNetintMapTake(DATA_REC(rec, btags), name);
}

bool SNetRecHasBTag( snet_record_t *rec, int name)
{
  return SNetRecGetBTag(rec, name) != -1;
}

/*****************************************************************************/

void SNetRecSetField( snet_record_t *rec, int name, snet_ref_t *val)
{
  SNetrefMapSet(DATA_REC(rec, fields), name, val);
}

snet_ref_t *SNetRecGetField( snet_record_t *rec, int name)
{
  return SNetInterfaceGet(DATA_REC( rec, interface_id))->copyfun(SNetrefMapGet(DATA_REC(rec, fields), name));
}

snet_ref_t *SNetRecTakeField( snet_record_t *rec, int name)
{
  return SNetrefMapTake(DATA_REC(rec, fields), name);
}

bool SNetRecHasField( snet_record_t *rec, int name)
{
  return SNetRecGetField(rec, name) != NULL;
}
