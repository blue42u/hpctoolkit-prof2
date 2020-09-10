// -*-Mode: C++;-*- // technically C99

// * BeginRiceCopyright *****************************************************
//
// $HeadURL$
// $Id$
//
// --------------------------------------------------------------------------
// Part of HPCToolkit (hpctoolkit.org)
//
// Information about sources of support for research and development of
// HPCToolkit is at 'hpctoolkit.org' and in 'README.Acknowledgments'.
// --------------------------------------------------------------------------
//
// Copyright ((c)) 2002-2020, Rice University
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// * Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
//
// * Neither the name of Rice University (RICE) nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// This software is provided by RICE and contributors "as is" and any
// express or implied warranties, including, but not limited to, the
// implied warranties of merchantability and fitness for a particular
// purpose are disclaimed. In no event shall RICE or contributors be
// liable for any direct, indirect, incidental, special, exemplary, or
// consequential damages (including, but not limited to, procurement of
// substitute goods or services; loss of use, data, or profits; or
// business interruption) however caused and on any theory of liability,
// whether in contract, strict liability, or tort (including negligence
// or otherwise) arising in any way out of the use of this software, even
// if advised of the possibility of such damage.
//
// ******************************************************* EndRiceCopyright *

//***************************************************************************
//
// Purpose:
//   Low-level types and functions for reading/writing id_tuples (each represent a unique profile)
//
// Description:
//   [The set of functions, macros, etc. defined in the file]
//
//***************************************************************************

//************************* System Include Files ****************************

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <sys/stat.h>

//*************************** User Include Files ****************************

#include <include/gcc-attr.h>

#include "../prof-lean/hpcio.h"
#include "../prof-lean/hpcio-buffer.h"
#include "../prof-lean/hpcfmt.h"
#include "../prof-lean/hpcrun-fmt.h"
#include "id-tuple.h"

//***************************************************************************
char* kindStr(const uint16_t kind)
{
  if(kind == IDTUPLE_SUMMARY){
    return "SUMMARY";
  }
  else if(kind == IDTUPLE_RANK){
    return "RANK";
  }
  else if(kind == IDTUPLE_THREAD){
    return "THREAD";
  }
  else{
    return "ERROR";
  }
}

//***************************************************************************
// single id tuple
//***************************************************************************

int 
id_tuple_fwrite(tms_id_tuple_t* x, FILE* fs)
{
    HPCFMT_ThrowIfError(hpcfmt_int2_fwrite(x->length, fs));
    for (uint j = 0; j < x->length; ++j) {
      HPCFMT_ThrowIfError(hpcfmt_int2_fwrite(x->ids[j].kind, fs));
      HPCFMT_ThrowIfError(hpcfmt_int8_fwrite(x->ids[j].index, fs));
    }
    return HPCFMT_OK;
}

int 
id_tuple_fread(tms_id_tuple_t* x, FILE* fs)
{
    HPCFMT_ThrowIfError(hpcfmt_int2_fread(&(x->length), fs));
    x->ids = (tms_id_t *) malloc(x->length * sizeof(tms_id_t)); 
    for (uint j = 0; j < x->length; ++j) {
      HPCFMT_ThrowIfError(hpcfmt_int2_fread(&(x->ids[j].kind), fs));
      HPCFMT_ThrowIfError(hpcfmt_int8_fread(&(x->ids[j].index), fs));
    }
    return HPCFMT_OK;
}

int 
id_tuple_fprint(tms_id_tuple_t* x, FILE* fs)
{
    fprintf(fs,"[");
    for (uint j = 0; j < x->length; ++j) {
      fprintf(fs,"(%s: %ld) ", kindStr(x->ids[j].kind), x->ids[j].index);
    }
    fprintf(fs,"]\n");
    return HPCFMT_OK;
}

void 
id_tuple_free(tms_id_tuple_t* x)
{
    free(x->ids);
    x->ids = NULL;
}


//***************************************************************************
// id tuple in thread.db
//***************************************************************************
int
id_tuples_tms_fwrite(uint32_t num_tuples, uint64_t id_tuples_size, tms_id_tuple_t* x, FILE* fs)
{
    HPCFMT_ThrowIfError(hpcfmt_int8_fwrite(id_tuples_size, fs));
    for (uint i = 0; i < num_tuples; ++i) {
        /*
        HPCFMT_ThrowIfError(hpcfmt_int2_fwrite(x[i].length, fs));
        for (uint j = 0; j < x[i].length; ++j) {
        HPCFMT_ThrowIfError(hpcfmt_int2_fwrite(x[i].ids[j].kind, fs));
        HPCFMT_ThrowIfError(hpcfmt_int8_fwrite(x[i].ids[j].index, fs));
        }
        */
        HPCFMT_ThrowIfError(id_tuple_fwrite(x+i,fs));
    }
    return HPCFMT_OK;
}

int
id_tuples_tms_fread(tms_id_tuple_t** x, uint64_t* id_tuples_size, uint32_t num_tuples,FILE* fs)
{
    HPCFMT_ThrowIfError(hpcfmt_int8_fread(id_tuples_size, fs));
    tms_id_tuple_t * id_tuples = (tms_id_tuple_t *) malloc(num_tuples*sizeof(tms_id_tuple_t));

    for (uint i = 0; i < num_tuples; ++i) {
        /*
        HPCFMT_ThrowIfError(hpcfmt_int2_fread(&(id_tuples[i].length), fs));
        id_tuples[i].ids = (tms_id_t *) malloc(id_tuples[i].length * sizeof(tms_id_t)); 
        for (uint j = 0; j < id_tuples[i].length; ++j) {
        HPCFMT_ThrowIfError(hpcfmt_int2_fread(&(id_tuples[i].ids[j].kind), fs));
        HPCFMT_ThrowIfError(hpcfmt_int8_fread(&(id_tuples[i].ids[j].index), fs));
        }
        */
        HPCFMT_ThrowIfError(id_tuple_fread(id_tuples+i, fs));
    }

    *x = id_tuples;
    return HPCFMT_OK;
}

int
id_tuples_tms_fprint(uint32_t num_tuples, uint64_t id_tuples_size, tms_id_tuple_t* x, FILE* fs)
{
  fprintf(fs,"[Id tuples for %d profiles, total size %ld\n", num_tuples, id_tuples_size);

  for (uint i = 0; i < num_tuples; ++i) {
    fprintf(fs,"  %d[", i);
    for (uint j = 0; j < x[i].length; ++j) {
      fprintf(fs,"(%s: %ld) ", kindStr(x[i].ids[j].kind), x[i].ids[j].index);
    }
    fprintf(fs,"]\n");
  }
  fprintf(fs,"]\n");
  return HPCFMT_OK;
}

void
id_tuples_tms_free(tms_id_tuple_t** x, uint32_t num_tuples)
{
  for (uint i = 0; i < num_tuples; ++i) {
    free((*x)[i].ids);
    (*x)[i].ids = NULL;
  }

  free(*x);
  *x = NULL;
}