/* $Id: bml.h,v 1.14 2005-04-30 11:17:51 ensonic Exp $
 */

#ifndef BML_H
#define BML_H

#include <setjmp.h>
#include <signal.h>
#include <pthread.h>

#undef BUZZ_MACHINE_LOADER_CPP
#include <BuzzMachineLoader.h>

//#ifdef __cplusplus
//extern "C" {
//#endif

#if defined (__GNUC__) && (__GNUC__ < 3)
#define DBG(str) printf("%s:%d:%s: %s\n",__FILE__,__LINE__,__FUNCTION__,str)
#else
#define DBG(str) printf("%s:%d:%s: %s\n",__FILE__,__LINE__,__func__,str)
#endif

/**
 * Display BML error message.
 *
 * Set via bml_set_error_function(), otherwise a BML-provided
 * default will print @a msg (plus a newline) to stderr.
 *
 * @param msg error message text (no newline at end).
 */
extern void (*bml_error_callback)(const char *msg);

/**
 * Set the @ref bml_error_callback for error message display.
 *
 * The BML library provides two built-in callbacks for this purpose:
 * default_bml_error_callback() and silent_bml_error_callback().
 */
void bml_set_error_function (void (*func)(const char *));

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

//#ifdef __cplusplus
//}
//#endif

#endif // BML_H
