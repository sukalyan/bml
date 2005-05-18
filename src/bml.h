/* $Id: bml.h,v 1.3 2005-05-18 19:55:52 ensonic Exp $
 */

#ifndef BML_H
#define BML_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#undef BUZZ_MACHINE_LOADER_CPP
#include <BuzzMachineLoader.h>

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE (!FALSE)
#endif

//#ifdef __cplusplus
//extern "C" {
//#endif

extern int bml_init(void (*sighandler)(int,siginfo_t*,void*));
extern void bml_done(void);
extern char *bml_convertpath(char *inpath);

// dll passthrough API method pointer types
typedef void (*BMSetMasterInfo)(long bpm, long tpb, long srat);
typedef BuzzMachine *(*BMNew)(char *bm_file_name);
typedef void (*BMInit)(BuzzMachine *bm);
typedef void (*BMFree)(BuzzMachine *bm);

typedef int (*BMGetMachineInfo)(BuzzMachine *bm, BuzzMachineProperty key, void *value);
typedef int (*BMGetGlobalParameterInfo)(BuzzMachine *bm,int index,BuzzMachineParameter key,void *value);
typedef int (*BMGetTrackParameterInfo)(BuzzMachine *bm,int index,BuzzMachineParameter key,void *value);
typedef int (*BMGetAttributeInfo)(BuzzMachine *bm,int index,BuzzMachineAttribute key,void *value);

typedef void * (*BMGetTrackParameterLocation)(BuzzMachine *bm,int track,int index);
typedef int  (*BMGetTrackParameterValue)(BuzzMachine *bm,int track,int index);
typedef void (*BMSetTrackParameterValue)(BuzzMachine *bm,int track,int index,int value);

typedef void * (*BMGetGlobalParameterLocation)(BuzzMachine *bm,int index);
typedef int  (*BMGetGlobalParameterValue)(BuzzMachine *bm,int index);
typedef void (*BMSetGlobalParameterValue)(BuzzMachine *bm,int index,int value);

typedef void * (*BMGetAttributeLocation)(BuzzMachine *bm,int index);
typedef int  (*BMGetAttributeValue)(BuzzMachine *bm,int index);
typedef void (*BMSetAttributeValue)(BuzzMachine *bm,int index,int value);

typedef void (*BMTick)(BuzzMachine *bm);
typedef int  (*BMWork)(BuzzMachine *bm,float *psamples, int numsamples, int const mode);
typedef int  (*BMWorkM2S)(BuzzMachine *bm,float *pin, float *pout, int numsamples, int const mode);
typedef void (*BMStop)(BuzzMachine *bm);

typedef void (*BMSetNumTracks)(BuzzMachine *bm, int num);

// dll passthrough API method pointers

extern BMSetMasterInfo bm_set_master_info;
extern BMNew bm_new;
extern BMInit bm_init;
extern BMFree bm_free;

extern BMGetMachineInfo bm_get_machine_info;
extern BMGetGlobalParameterInfo bm_get_global_parameter_info;
extern BMGetTrackParameterInfo bm_get_track_parameter_info;
extern BMGetAttributeInfo bm_get_attribute_info;

extern BMGetTrackParameterLocation bm_get_track_parameter_location;
extern BMGetTrackParameterValue bm_get_track_parameter_value;
extern BMSetTrackParameterValue bm_set_track_parameter_value;

extern BMGetGlobalParameterLocation bm_get_global_parameter_location;
extern BMGetGlobalParameterValue bm_get_global_parameter_value;
extern BMSetGlobalParameterValue bm_set_global_parameter_value;

extern BMGetAttributeLocation bm_get_attribute_location;
extern BMGetAttributeValue bm_get_attribute_value;
extern BMSetAttributeValue bm_set_attribute_value;

extern BMTick bm_tick;
extern BMWork bm_work;
extern BMWorkM2S bm_work_m2s;
extern BMStop bm_stop;

extern BMSetNumTracks bm_set_num_tracks;

// dll wrapper API functions
/*
extern void bm_set_master_info(long bpm, long tpb, long srat);
extern BuzzMachine *bm_new(char *bm_file_name);
extern void bm_init(BuzzMachine *bm);
extern void bm_free(BuzzMachine *bm);

extern int bm_get_machine_info(BuzzMachine *bm, BuzzMachineProperty key, void *value);
extern int bm_get_global_parameter_info(BuzzMachine *bm,int index,BuzzMachineParameter key,void *value);
extern int bm_get_track_parameter_info(BuzzMachine *bm,int index,BuzzMachineParameter key,void *value);
extern int bm_get_attribute_info(BuzzMachine *bm,int index,BuzzMachineAttribute key,void *value);

extern void *bm_get_track_parameter_location(BuzzMachine *bm,int track,int index);
extern int bm_get_track_parameter_value(BuzzMachine *bm,int track,int index);
extern void bm_set_track_parameter_value(BuzzMachine *bm,int track,int index,int value);

extern void *bm_get_global_parameter_location(BuzzMachine *bm,int index);
extern int bm_get_global_parameter_value(BuzzMachine *bm,int index);
extern void bm_set_global_parameter_value(BuzzMachine *bm,int index,int value);

extern void *bm_get_attribute_location(BuzzMachine *bm,int index);
extern int bm_get_attribute_value(BuzzMachine *bm,int index);
extern void bm_set_attribute_value(BuzzMachine *bm,int index,int value);

extern void bm_tick(BuzzMachine *bm);
extern int bm_work(BuzzMachine *bm,float *psamples, int numsamples, int const mode);
extern int bm_work_m2s(BuzzMachine *bm,float *pin, float *pout, int numsamples, int const mode);
extern void bm_stop(BuzzMachine *bm);

extern void bm_set_num_tracks(BuzzMachine *bm, int num);
*/

//#ifdef __cplusplus
//}
//#endif

#endif // BML_H
