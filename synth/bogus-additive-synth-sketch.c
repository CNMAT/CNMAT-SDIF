#if 0

/* SDIFU_TRCtoSamples --

   Synthesizes "num_samples" samples and puts them into "samples" at
   a rate of "sample_rate".

   "current" and "next" are the SDIF frames we are ramping between.

   "state" is unused in this case.

   NOTE: to avoid phase discontinuity problems, this function ignores
   all phase information for a sinusoid except for the first time this
   information is available (i.e., at a birth).  To pass this information
   from one synthesis frame to the next, the arrived-at phase is
   *written*into* "next".  This will not happen if "next" is NULL. */


int
SDIFU_TRCtoSamples(int num_samples,
		   sdif_float32 *samples, sdif_float32 sample_rate,
                   SDIFFrame *current, SDIFFrame *next, void *state);





int
SDIFU_TRCtoSamples(int num_samples, float32 *samples, float32 sample_rate,
                   SDIFFrame *current, SDIFFrame *next, void *state) {

    /* This version pays attention to each partial's initial phase, 
       then discards all subsequent phase info.  Instead it has its
       own idea of each oscillator's phase.  Hack: write the phase we
       reach at the end of this interval into the phase field of the
       next frame. */

    int i, j;
    float32 pos;
    float32 t, current_time, next_time, time_span;
    ResultCode result = OK;


    current_time = current->time;
    next_time = next->time;
    time_span = next_time - current_time;

    /* Figure out categories:
       Normal: exists in both frames
       Good birth: not in current; 0 amp in next
       Bad birth:  not in current; non-0 amp in next
       Good death: 0 amp in current; not in next
       Bad death: non-0 amp in current; not in next */

    foo = MakeADataStructure();

    for (i = 0; i < current->num; ++i) {
	Insert(foo, current->index[i],
	       current->amp[i] == 0.0 ? GOOD_DEATH : BAD_DEATH,
	       current->phase[i], current->freq[i], current->amp[i],
	       current->freq[i], 0.);
    }

    for (i = 0; i < next->num; ++i) {
	if (Exists(foo, next->index[i])) {
	    Change(foo, next->index[i], NORMAL, next->freq[i], next->amp[i]);
	} else {
	    Insert(foo, next->index[i],
		   next->amp[i] == 0.0 ? GOOD_BIRTH : BAD_BIRTH,
		   ReverseEngineerPhase(next->phase[i], next->freq[i], time_span),
		   next->freq[i], 0., next->freq[i], next->amp[i]);
	}
    }

    int num_sinusoids = Count(foo);

    t = current_time;
    for (i=0; i < num_samples; i++) {
	time += 1 / sample_rate;
	pos = (t - current_time) / (next_time - current_time);
	samples[i] = 0;

	Iterate(foo, thisSine) {
	    switch (thisSine->category) {

		case BAD_DEATH:
		set_error_code(ESDIFU_BAD_SINUSOID_DEATH);
		ResultCode = SDIFU_BAD_DEATH;
		goto normal;
		break;	/* Does this do anything?  Amar says no.*/

		case BAD_BIRTH:
		set_error_code(ESDIFU_BAD_SINUSOID_BIRTH);
		ResultCode = SDIFU_BAD_BIRTH;
		/* fall through */

		case NORMAL:
		normal:
		samples[i] += sin(GetPhase(thisSine)) * 
			      (*myAmpInterpolator)(pos, GetInitAmp(thisSine),
						   GetEndAmp(thisSine));
		UpdatePhase(thisSine,
			    fmod(GetPhase(thisSine) + 
				 (*myFreqInterpolator)(pos, 
						       GetInitFreq(thisSine),
						       GetEndFreq(thisSine)),
				 TWOPI));
		break;

		case GOOD_BIRTH: case GOOD_DEATH:
		/* Output nothing, because its going between silent and not there
		   or vice-versa. */
		break;
	    }
	}
    }

    /* Here's the hack: */
    for (i = 0; i < next->num; ++i) {
#ifndef NDEBUG
	if (GetCategory(next->index[i]) == GOOD_BIRTH || 
	    GetCategory(next->index[i]) == BAD_BIRTH) {
	    if (fabs(next->phase[i], GetPhaseTheOtherWay(foo, next->index[i])) > 
		ACCEPTABLE_PHASE_EPSILON) {
		fatal_error("This program is wrong.");
	    }
	}
#endif

	next->phase[i] = GetPhaseTheOtherWay(foo, next->index[i]);
    }

    Discard(foo);
    return result;
}


static void
Insert(DataStructure d, float32 index, Category c,
       float32 CurrentPhase, float32 initFreq, float32 initAmp,
       float32 endFreq, float32 endAmp) {
    ...
}

#endif

