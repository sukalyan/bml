/* Buzz Machine Loader
 * Copyright (C) 2006 Buzztrax team <buzztrax-devel@buzztrax.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#define BUZZ_MACHINE_LOADER_CPP
#define BUZZ_MACHINE_LOADER

#ifdef WIN32
#include "stdafx.h"
#include <windows.h>
#else
#include "windef.h"
#include <dlfcn.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include "debug.h"

#include "BuzzMachineCallbacks.h"
#include "BuzzMachineCallbacksPre12.h"
#include "MachineDataImpl.h"
#include "CMachine.h"
#include "BuzzMachineLoader.h"
#include "dsplib.h"

#ifdef _MSC_VER
#define DE __declspec(dllexport)
#else
#define DE 
#endif

// buzz emulation code

typedef void (*CMIInitPtr)(CMachineInterface *_this, CMachineDataInput * const pi);

// globals

CMasterInfo master_info;

// prototypes

extern "C" DE void bm_free(BuzzMachine *bm);
extern "C" DE BuzzMachine *bm_new(BuzzMachineHandle *bmh);
extern "C" DE void bm_init(BuzzMachine *bm, unsigned long blob_size, unsigned char *blob_data);

extern "C" DE void bm_set_attribute_value(BuzzMachine *bm,int index,int value);
extern "C" DE void bm_set_global_parameter_value(BuzzMachine *bm,int index,int value);
extern "C" DE void bm_set_track_parameter_value(BuzzMachine *bm,int track,int index,int value);

// helpers


// global API

extern "C" DE void bm_set_logger(DebugLoggerFunc func) {
    debug_log_func=func;
}

extern "C" DE void bm_set_master_info(long bpm, long tpb, long srat) {
  master_info.BeatsPerMin=bpm;        /*   120 */
  master_info.TicksPerBeat=tpb;        /*     4 */
  master_info.SamplesPerSec=srat;    /* 44100 */
  master_info.SamplesPerTick=(int)((60*master_info.SamplesPerSec)/(master_info.BeatsPerMin*master_info.TicksPerBeat));
  master_info.PosInTick=0; /*master_info.SamplesPerTick-1;*/
  master_info.TicksPerSec=(float)master_info.SamplesPerSec/(float)master_info.SamplesPerTick;
#ifdef _MSC_VER
  DSP_Init(master_info.SamplesPerSec);
#else
  /* todo: we need a way to link fastcall stuff from mingw-cross
   * undefined symbol: _Z8DSP_Initi
   * right now we link statically and call this from the plugins.
   */
  //DSP_Init(master_info.SamplesPerSec);
#endif
}


// library api

extern "C" DE void bm_close(BuzzMachineHandle *bmh) {
    if(bmh) {
        if(bmh->bm) {
            bm_free(bmh->bm);
        }
        if(bmh->lib_name) {
            free(bmh->lib_name);
        }
        if(bmh->h) {
#ifdef WIN32
            FreeLibrary((HMODULE)(bmh->h));
#else
            dlclose(bmh->h);
#endif
            DBG("  dll unloaded\n");
        }
        free(bmh);
    }
}

extern "C" DE BuzzMachineHandle *bm_open(char *bm_file_name) {
    BuzzMachineHandle *bmh=(BuzzMachineHandle *)calloc(sizeof(BuzzMachineHandle),1);
    GetInfoPtr GetInfo=NULL;
    CreateMachinePtr CreateMachine=NULL;

#ifdef WIN32
    bmh->h=(void*)LoadLibraryA(bm_file_name);
#else
    bmh->h=dlopen(bm_file_name,RTLD_LAZY);
#endif
    if(!bmh->h) {
#ifdef WIN32
        DBG1("  failed to load machine dll from \"%s\"\n",bm_file_name);
#else
        DBG2("  failed to load machine dll from \"%s\": %s\n",bm_file_name,dlerror());
#endif
        bm_close(bmh);
        return(NULL);
    }
    DBG1("  dll %s loaded\n",bm_file_name);
    bmh->lib_name=strdup(bm_file_name);

    //-- get the two dll entries
#ifdef WIN32
    GetInfo      =(GetInfoPtr      )GetProcAddress((HMODULE)(bmh->h),"GetInfo");
    CreateMachine=(CreateMachinePtr)GetProcAddress((HMODULE)(bmh->h),"CreateMachine");
#else
    GetInfo      =(GetInfoPtr      )dlsym(bmh->h,"GetInfo");
    CreateMachine=(CreateMachinePtr)dlsym(bmh->h,"CreateMachine");
#endif
    if(!GetInfo) {
        DBG("  failed to connect to GetInfo method\n");
        bm_close(bmh);
        return(NULL);
    }
    if(!CreateMachine) {
        DBG("  failed to connect to GreateMachine method\n");
        bm_close(bmh);
        return(NULL);
    }
    bmh->CreateMachine=CreateMachine;
    DBG("  symbols connected\n");

    //-- call GetInfo
    bmh->machine_info=GetInfo();
    DBG("  GetInfo() called\n");

    //-- apply fixes
    if(!bmh->machine_info->minTracks) {
        if(bmh->machine_info->numTrackParameters) {
            DBG("!! buggy machine, numTrackParams>0, but minTracks=0\n");
            bmh->machine_info->numTrackParameters=0;
        }
        if(bmh->machine_info->maxTracks) {
            DBG("!! buggy machine, maxTracks>0, but minTracks=0\n");
            bmh->machine_info->maxTracks=0;
        }
    }
    
    //-- we need to temporarily create an instance to query extra data
    if((bmh->bm=bm_new(bmh))) {
      bm_init(bmh->bm,0,NULL);
      if(bmh->bm->mdkHelper) {
        if(bmh->bm->mdkHelper->numChannels) {
          bmh->mdk_num_channels=bmh->bm->mdkHelper->numChannels;
        }
      }
    }
    else {
      DBG("  CreateMachine() failed\n");
      bm_close(bmh);
      return(NULL);
    }
    
    return(bmh);
}

extern "C" DE int bm_get_machine_info(BuzzMachineHandle *bmh,BuzzMachineProperty key,void *value) {
    int ret=TRUE;
    int *ival;
    const char **sval;

    if(!value) return(FALSE);

    ival=(int *)value;
    sval=(const char **)value;
    switch(key) {
        case BM_PROP_TYPE:                *ival=bmh->machine_info->Type;break;
        case BM_PROP_VERSION:             *ival=bmh->machine_info->Version;break;
        case BM_PROP_FLAGS:               *ival=bmh->machine_info->Flags;break;
        case BM_PROP_MIN_TRACKS:          *ival=bmh->machine_info->minTracks;break;
        case BM_PROP_MAX_TRACKS:          *ival=bmh->machine_info->maxTracks;break;
        case BM_PROP_NUM_GLOBAL_PARAMS:   *ival=bmh->machine_info->numGlobalParameters;break;
        case BM_PROP_NUM_TRACK_PARAMS:    *ival=bmh->machine_info->numTrackParameters;break;
        case BM_PROP_NUM_ATTRIBUTES:      *ival=bmh->machine_info->numAttributes;break;
        case BM_PROP_NAME:                *sval=bmh->machine_info->Name;break;
        case BM_PROP_SHORT_NAME:          *sval=bmh->machine_info->ShortName;break;
        case BM_PROP_AUTHOR:              *sval=bmh->machine_info->Author;break;
        case BM_PROP_COMMANDS:            *sval=bmh->machine_info->Commands;break;
        case BM_PROP_DLL_NAME:            *sval=bmh->lib_name;break;
        case BM_PROP_NUM_INPUT_CHANNELS:
          //*ival=(bm->mdkHelper && bm->mdkHelper->numChannels)?bm->mdkHelper->numChannels:1;
          if(bmh->mdk_num_channels) {
            *ival=bmh->mdk_num_channels;
          }
          else {
            *ival=1;
          }
          break;
        case BM_PROP_NUM_OUTPUT_CHANNELS:
          //*ival=(bm->mdkHelper && bm->mdkHelper->numChannels==2)?2:((bm->machine_info->Flags&MIF_MONO_TO_STEREO)?2:1);
          if(bmh->mdk_num_channels==2) {
            *ival=2;
          }
          else {
            if(bmh->machine_info->Flags&MIF_MONO_TO_STEREO) {
              *ival=2;
            }
            else {
              *ival=1;
            }
          }
          break;
        default: ret=FALSE;
    }
    return(ret);
}

static int bm_get_parameter_info(BuzzMachineHandle *bmh,int index,BuzzMachineParameter key,void *value) {
    int ret=TRUE;
    int *ival;
    const char **sval;

    ival=(int *)value;
    sval=(const char **)value;
    switch(key) {
        case BM_PARA_TYPE:            *ival=bmh->machine_info->Parameters[index]->Type;break;
        case BM_PARA_NAME:            *sval=bmh->machine_info->Parameters[index]->Name;break;
        case BM_PARA_DESCRIPTION:     *sval=bmh->machine_info->Parameters[index]->Description;break;
        case BM_PARA_MIN_VALUE:       *ival=bmh->machine_info->Parameters[index]->MinValue;break;
        case BM_PARA_MAX_VALUE:       *ival=bmh->machine_info->Parameters[index]->MaxValue;break;
        case BM_PARA_NO_VALUE:        *ival=bmh->machine_info->Parameters[index]->NoValue;break;
        case BM_PARA_FLAGS:           *ival=bmh->machine_info->Parameters[index]->Flags;break;
        case BM_PARA_DEF_VALUE:       *ival=bmh->machine_info->Parameters[index]->DefValue;break;
        default: ret=FALSE;
    }
    return(ret);
}

extern "C" DE int bm_get_global_parameter_info(BuzzMachineHandle *bmh,int index,BuzzMachineParameter key,void *value) {
    if(!value) return(FALSE);
    if(!(index<bmh->machine_info->numGlobalParameters)) return(FALSE);

    return(bm_get_parameter_info(bmh,index,key,value));
}

extern "C" DE int bm_get_track_parameter_info(BuzzMachineHandle *bmh,int index,BuzzMachineParameter key,void *value) {
    if(!value) return(FALSE);
    if(!(index<bmh->machine_info->numTrackParameters)) return(FALSE);

    return(bm_get_parameter_info(bmh,(bmh->machine_info->numGlobalParameters+index),key,value));
}

extern "C" DE int bm_get_attribute_info(BuzzMachineHandle *bmh,int index,BuzzMachineAttribute key,void *value) {
    int ret=TRUE;
    int *ival;
    const char **sval;

    if(!value) return(FALSE);
    if(!(index<bmh->machine_info->numAttributes)) return(FALSE);

    ival=(int *)value;
    sval=(const char **)value;
    switch(key) {
        case BM_ATTR_NAME:        *sval=bmh->machine_info->Attributes[index]->Name;break;
        case BM_ATTR_MIN_VALUE:   *ival=bmh->machine_info->Attributes[index]->MinValue;break;
        case BM_ATTR_MAX_VALUE:   *ival=bmh->machine_info->Attributes[index]->MaxValue;break;
        case BM_ATTR_DEF_VALUE:   *ival=bmh->machine_info->Attributes[index]->DefValue;break;
        default: ret=FALSE;
    }
    return(ret);
}

extern "C" DE char const *bm_describe_global_value(BuzzMachineHandle *bmh,int const param,int const value) {
    static const char *empty="";

    if(!(param<bmh->machine_info->numGlobalParameters)) return(empty);

    //DBG2("(param=%d,value=%d)\n",param,value);
    return(bmh->bm->machine_iface->DescribeValue(param,value));
}

extern "C" DE char const *bm_describe_track_value(BuzzMachineHandle *bmh,int const param,int const value) {
    static const char *empty="";

    if(!(param<bmh->machine_info->numTrackParameters)) return(empty);

    //DBG2("(param=%d,value=%d)\n",param,value);
    return(bmh->bm->machine_iface->DescribeValue(bmh->machine_info->numGlobalParameters+param,value));
}


// instance api

extern "C" DE void bm_free(BuzzMachine *bm) {
    if(bm) {
        CMICallbacks *callbacks = bm->callbacks;
        DBG("freeing\n");
        delete bm->machine_iface;
        delete bm->machine;
        
        if(callbacks) {
            int version = bm->machine_info->Version;
            DBG1("freeing callbacks 0x%04x\n",version);
        }
        free(bm);
    }
}

extern "C" DE BuzzMachine *bm_new(BuzzMachineHandle *bmh) {
    BuzzMachine *bm=(BuzzMachine *)calloc(sizeof(BuzzMachine),1);

    // copy some fields from bmh for convinient access
    bm->bmh=bmh;
    bm->machine_info=bm->bmh->machine_info;

    // call CreateMachine
    bm->machine_iface=bm->bmh->CreateMachine();
    DBG("  CreateMachine() called\n");

    // we need to create a CMachine object
    bm->machine=new CMachine(bm->machine_iface,bm->machine_info);
    
    // not callbacks set by host so far
    bm->host_callbacks = NULL;

    DBG1("  mi-version 0x%04x\n",bm->machine_info->Version);
    if((bm->machine_info->Version & 0xff) < 15) {
      bm->callbacks=(CMICallbacks *)new BuzzMachineCallbacksPre12(bm->machine,bm->machine_iface,bm->machine_info,&bm->host_callbacks);
      DBG("  old callback instance created\n");
    }
    else {
      bm->callbacks=(CMICallbacks *)new BuzzMachineCallbacks(bm->machine,bm->machine_iface,bm->machine_info,&bm->host_callbacks);
      DBG("  callback instance created\n");
    }

    // FIXME: should we do this earlier?
    bm->machine_iface->pMasterInfo=&master_info;
    bm->machine_iface->pCB=bm->callbacks;

    return(bm);
}

extern "C" DE void bm_init(BuzzMachine *bm, unsigned long blob_size, unsigned char *blob_data) {
    int i,j;

    DBG2("  bm_init(bm,%ld,%p)\n",blob_size,blob_data);

    // initialise attributes
    for(i=0;i<bm->machine_info->numAttributes;i++) {
        bm_set_attribute_value(bm,i,bm->machine_info->Attributes[i]->DefValue);
    }
    DBG("  attributes initialized\n");

    // create the machine data input
    CMachineDataInput * pcmdii = NULL;

    if (blob_size && blob_data) {
      pcmdii = new CMachineDataInputImpl(blob_data, blob_size);
    }
   
    // call Init (this also calles mdkHelper::Init()
    bm->machine_iface->Init(pcmdii);
    DBG("  CMachineInterface::Init() called\n");
    // create and get mdk implementation
    if((bm->machine_info->Version & 0xff) >= 15) {
      // imho mdk only works with for machines that use buzz 1.2 api
      // we need to avoid creating mdkimpl here, if the machine is an mdkmachine
      // its already created
      // CMDKMachineInterface::Init() also sets CMachineInterfaceEx
      if(((BuzzMachineCallbacks *)bm->callbacks)->machine_ex) {
        bm->mdkHelper = (CMDKImplementation*)bm->callbacks->GetNearestWaveLevel(-1,-1);
        DBG1("  numInputChannels=%d\n",(bm->mdkHelper)?bm->mdkHelper->numChannels:0);
        // if numChannels=0, its not a mdk-machine and numChannels=1
      }
    }

    // call AttributesChanged always
    //if(bm->machine_info->numAttributes>0) {
		bm->machine_iface->AttributesChanged();
        DBG("  CMachineInterface::AttributesChanged() called\n");
    //}

    // call SetNumTracks
    //DBG1("  CMachineInterface::SetNumTracks(%d)\n",bm->machine_info->minTracks);
	// calling this without the '-1' crashes: Automaton Parametric EQ.dll
	//bm->machine_iface->SetNumTracks(bm->machine_info->minTracks-1);
	bm->machine_iface->SetNumTracks(bm->machine_info->minTracks);
    DBG1("  CMachineInterface::SetNumTracks(%d) called\n",bm->machine_info->minTracks);

    // FIXME: CyanPhase DTMF-1 access gval.xxx in mi::Init
    // -> so we might need to call these before

    // initialise global parameters (DefValue or NoValue, Buzz seems to use NoValue)
    for(i=0;i<bm->machine_info->numGlobalParameters;i++) {
        if(bm->machine_info->Parameters[i]->Flags&MPF_STATE) {
            bm_set_global_parameter_value(bm,i,bm->machine_info->Parameters[i]->DefValue);
        }
        else {
            bm_set_global_parameter_value(bm,i,bm->machine_info->Parameters[i]->NoValue);
        }
    }
    DBG("  global parameters initialized\n");
    // initialise track parameters
    if((bm->machine_info->minTracks>0) && (bm->machine_info->maxTracks>0)) {
        DBG3(" need to initialize %d track params for tracks: %d...%d\n",bm->machine_info->numTrackParameters,bm->machine_info->minTracks,bm->machine_info->maxTracks);
        for(j=0;j<bm->machine_info->maxTracks;j++) {
            for(i=0;i<bm->machine_info->numTrackParameters;i++) {
                if(bm->machine_info->Parameters[bm->machine_info->numGlobalParameters+i]->Flags&MPF_STATE) {
                    bm_set_track_parameter_value(bm,j,i,bm->machine_info->Parameters[bm->machine_info->numGlobalParameters+i]->DefValue);
                }
                else {
                    bm_set_track_parameter_value(bm,j,i,bm->machine_info->Parameters[bm->machine_info->numGlobalParameters+i]->NoValue);
                }
            }
        }
    }
    DBG("  track parameters initialized\n");

    /* we've given the machine the initial global- and track-parameters,
     * and the attributes, give it a tick
     * - tick HERE rather than later, to give (for example) the Master machine a
     *   chance to update its state before any other machines need to rely on it
     * - tick AFTER AttributesChanged, and after we've set initial track and
     *   global data for machine)
     */
    bm->machine_iface->Tick();
    DBG("  CMachineInterface::Tick() called\n");

    if(bm->machine_info->Flags&MIF_USES_LIB_INTERFACE) {
        DBG(" MIF_USES_LIB_INTERFACE");
        FIXME;
    }
}

extern "C" DE void * bm_get_track_parameter_location(BuzzMachine *bm,int track,int index) {
    void *res=NULL;

    if(!(track<bm->machine_info->maxTracks)) return(0);
    if(!(index<bm->machine_info->numTrackParameters)) return(0);
    if(!(bm->machine_iface->TrackVals)) return(0);

    byte *ptr=(byte *)bm->machine_iface->TrackVals;
    if(!ptr) {
      DBG(" -> machine->TrackVals is NULL!\n");
    }

    // @todo prepare pointer array in bm_init
    for(int j=0;j<=track;j++) {
        for(int i=0;i<bm->machine_info->numTrackParameters;i++) {
            switch(bm->machine_info->Parameters[bm->machine_info->numGlobalParameters+i]->Type) {
                case pt_note:
                case pt_switch:
                case pt_byte:
                    if((j==track) && (i==index)) res=(void *)ptr;
                    else ptr++;
                    break;
                case pt_word:
                    if((j==track) && (i==index)) res=(void *)ptr;
                    else ptr+=2;
                    break;
            }
        }
    }
    return(res);
}

extern "C" DE int bm_get_track_parameter_value(BuzzMachine *bm,int track,int index) {
    int value=0;
    if(!(index<bm->machine_info->numTrackParameters)) return(0);
    if(!(bm->machine_iface->TrackVals)) return(0);

    void *ptr=bm_get_track_parameter_location(bm,track,index);
    switch(bm->machine_info->Parameters[bm->machine_info->numGlobalParameters+index]->Type) {
        case pt_note:
        case pt_switch:
        case pt_byte:
            value=(int)(*((byte *)ptr));
            break;
        case pt_word:
            value=(int)(*((word *)ptr));
            break;
    }
    return(value);
}

extern "C" DE void bm_set_track_parameter_value(BuzzMachine *bm,int track,int index,int value) {
    if(!(index<bm->machine_info->numTrackParameters)) return;
    if(!(bm->machine_iface->TrackVals)) return;

    void *ptr=bm_get_track_parameter_location(bm,track,index);
    switch(bm->machine_info->Parameters[bm->machine_info->numGlobalParameters+index]->Type) {
        case pt_note:
        case pt_switch:
        case pt_byte:
            (*((byte *)ptr))=(byte)value;
            break;
        case pt_word:
            (*((word *)ptr))=(word)value;
            break;
    }
}

extern "C" DE void * bm_get_global_parameter_location(BuzzMachine *bm,int index) {
    void *res=NULL;

    if(!(index<bm->machine_info->numGlobalParameters)) return(0);
    if(!(bm->machine_iface->GlobalVals)) return(0);

    byte *ptr=(byte *)bm->machine_iface->GlobalVals;
    if(!ptr) {
      DBG(" -> machine->GlobalVals is NULL!\n");
    }

    // @todo prepare pointer array in bm_init
    for(int i=0;i<=index;i++) {
        switch(bm->machine_info->Parameters[i]->Type) {
            case pt_note:
            case pt_switch:
            case pt_byte:
                if(i==index) res=(void *)ptr;
                else ptr++;
                break;
            case pt_word:
                if(i==index) res=(void *)ptr;
                else ptr+=2;
                break;
        }
    }
    return(res);
}

extern "C" DE int bm_get_global_parameter_value(BuzzMachine *bm,int index) {
    int value=0;
    if(!(index<bm->machine_info->numGlobalParameters)) return(0);
    if(!(bm->machine_iface->GlobalVals)) return(0);

    void *ptr=bm_get_global_parameter_location(bm,index);
    switch(bm->machine_info->Parameters[index]->Type) {
        case pt_note:
        case pt_switch:
        case pt_byte:
            value=(int)(*((byte *)ptr));
            break;
        case pt_word:
            value=(int)(*((word *)ptr));
            break;
    }
    return(value);
}

extern "C" DE void bm_set_global_parameter_value(BuzzMachine *bm,int index,int value) {
    if(!(index<bm->machine_info->numGlobalParameters)) return;
    if(!(bm->machine_iface->GlobalVals)) return;

    void *ptr=bm_get_global_parameter_location(bm,index);
    //printf("%s: index=%d, GlobalVals :%p, %p\n",__PRETTY_FUNCTION__,index,bm->machine_iface->GlobalVals,ptr);
    switch(bm->machine_info->Parameters[index]->Type) {
        case pt_note:
        case pt_switch:
        case pt_byte:
            (*((byte *)ptr))=(byte)value;
            break;
        case pt_word:
            (*((word *)ptr))=(word)value;
            break;
    }
}

extern "C" DE void * bm_get_attribute_location(BuzzMachine *bm,int index) {
    if(!(index<bm->machine_info->numAttributes)) return(0);
    if(!(bm->machine_iface->AttrVals)) return(0);

    return((void *)&bm->machine_iface->AttrVals[index]);
}

extern "C" DE int bm_get_attribute_value(BuzzMachine *bm,int index) {
    if(!(index<bm->machine_info->numAttributes)) return(0);
    if(!(bm->machine_iface->AttrVals)) return(0);

    return(bm->machine_iface->AttrVals[index]);
}

extern "C" DE void bm_set_attribute_value(BuzzMachine *bm,int index,int value) {
    if(!(index<bm->machine_info->numAttributes)) return;
    if(!(bm->machine_iface->AttrVals)) return;

    bm->machine_iface->AttrVals[index]=value;
}

extern "C" DE void bm_attributes_changed(BuzzMachine *bm) {
    // call AttributesChanged
    // FIXME: should we call this always?
    if(bm->machine_info->numAttributes>0) {
		bm->machine_iface->AttributesChanged();
        DBG("  CMachineInterface::AttributesChanged() called\n");
    }
}

extern "C" DE void bm_tick(BuzzMachine *bm) {
    bm->machine_iface->Tick();
    // FIXME: do we need to bump that from somewhere
    //DBG1("master_info.PosInTick = %d\n", master_info.PosInTick);
}

extern "C" DE bool bm_work(BuzzMachine *bm,float *psamples, int numsamples, int const mode) {
    bool res=0;

	res=bm->machine_iface->Work(psamples,numsamples,mode);
    return(res);
}

extern "C" DE bool bm_work_m2s(BuzzMachine *bm, float *pin, float *pout, int numsamples, int const mode) {
    bool res=0;

	res=bm->machine_iface->WorkMonoToStereo(pin,pout,numsamples,mode);
    return(res);
}

extern "C" DE void bm_stop(BuzzMachine *bm) {
	bm->machine_iface->Stop();
}

/*
virtual void Command(int const i) {}
*/
extern "C" DE void bm_set_num_tracks(BuzzMachine *bm, int num) {
	DBG1("(num=%d)\n",num);
	bm->machine_iface->SetNumTracks(num);
	// we don't need to initialize the track params, as the max-num of tracks is already initialized in bm_init()
    // dunno if some machines would require this
}

/*
virtual void MuteTrack(int const i) {}
virtual bool IsTrackMuted(int const i) const { return false; }
*/

extern "C" DE void bm_set_callbacks(BuzzMachine *bm, CHostCallbacks *callbacks) {
    bm->host_callbacks=callbacks;
}

