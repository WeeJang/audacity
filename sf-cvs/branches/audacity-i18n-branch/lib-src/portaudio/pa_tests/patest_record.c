/*
 * patest_record.c
 * Record input into an array.
 * Save array to a file.
 * Playback recorded data.
 *
 * Author: Phil Burk  http://www.softsynth.com
 *
 * This program uses the PortAudio Portable Audio Library.
 * For more information see: http://www.portaudio.com
 * Copyright (c) 1999-2000 Ross Bencina and Phil Burk
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * Any person wishing to distribute modifications to the Software is
 * requested to send the modifications to the original developer so that
 * they can be incorporated into the canonical version.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include "portaudio.h"

/* #define SAMPLE_RATE  (17932) /* Test failure to open with this value. */
#define SAMPLE_RATE  (44100)
#define NUM_SECONDS     (5)

/* Select sample format. */
#if 0
#define PA_SAMPLE_TYPE  paFloat32
typedef float SAMPLE;
#elif 0
#define PA_SAMPLE_TYPE  paInt16
typedef short SAMPLE;
#else
#define PA_SAMPLE_TYPE  paUInt8
typedef unsigned char SAMPLE;
#endif

typedef struct
{
	int          frameIndex;  /* Index into sample array. */
	int          maxFrameIndex;
	int          samplesPerFrame;
	SAMPLE      *recordedSamples;
} paTestData;
/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int recordCallback(	void *inputBuffer, void *outputBuffer,
						unsigned long framesPerBuffer,
				        PaTimestamp outTime, void *userData )
{
	paTestData *data = (paTestData*)userData;
	SAMPLE *rptr = (SAMPLE*)inputBuffer;
	SAMPLE *wptr = &data->recordedSamples[data->frameIndex * data->samplesPerFrame];
	long framesToCalc;
	long i;
	int finished;
	unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;
		
	(void) outputBuffer; /* Prevent unused variable warnings. */
	(void) outTime;
	
	if( framesLeft < framesPerBuffer )
	{
		framesToCalc = framesLeft;
		finished = 1;
	}
	else
	{
		framesToCalc = framesPerBuffer;
		finished = 0;
	}
	if( inputBuffer == NULL )
	{
	   	for( i=0; i<framesToCalc; i++ )
		{
			*wptr++ = 0;		/* left */
			*wptr++ = 0;		/* right */
		}
	}
	else
	{
		for( i=0; i<framesToCalc; i++ )
		{
			*wptr++ = *rptr++;		/* left */
			*wptr++ = *rptr++;		/* right */
		}
	}
	data->frameIndex += framesToCalc;
	return finished;
}

/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int playCallback(	void *inputBuffer, void *outputBuffer,
						unsigned long framesPerBuffer,
				        PaTimestamp outTime, void *userData )
{
	paTestData *data = (paTestData*)userData;
	SAMPLE *rptr = &data->recordedSamples[data->frameIndex * data->samplesPerFrame];
	SAMPLE *wptr = (SAMPLE*)outputBuffer;
	unsigned int i;
	int finished;
	unsigned int framesLeft = data->maxFrameIndex - data->frameIndex;
	(void) inputBuffer; /* Prevent unused variable warnings. */
	(void) outTime;
	
	if( framesLeft < framesPerBuffer )
	{
		/* final buffer... */     
		for( i=0; i<framesLeft; i++ )
		{
			*wptr++ = *rptr++;		/* left */
			*wptr++ = *rptr++;		/* right */
		}
		for( ; i<framesPerBuffer; i++ )
		{
			*wptr++ = 0;		/* left */
			*wptr++ = 0;		/* right */
		}
		data->frameIndex += framesLeft;
		finished = 1;
	}
	else
	{
		for( i=0; i<framesPerBuffer; i++ )
		{
			*wptr++ = *rptr++;		/* left */
			*wptr++ = *rptr++;		/* right */
		}
		data->frameIndex += framesPerBuffer;
		finished = 0;
	}
	return finished;
}

/*******************************************************************/
int main(void);
int main(void)
{
	PortAudioStream *stream;
	PaError    err;
	paTestData data;
	int        i;
	int        totalFrames;
	int        numSamples;
	int        numBytes;
	SAMPLE     max, average, val;
	printf("patest_record.c\n"); fflush(stdout);
	
	data.maxFrameIndex = totalFrames = NUM_SECONDS * SAMPLE_RATE; /* Record for a few seconds. */
	data.frameIndex = 0;
	data.samplesPerFrame = 2;
	numSamples = totalFrames * data.samplesPerFrame;

	numBytes = numSamples * sizeof(SAMPLE);
	data.recordedSamples = (SAMPLE *) malloc( numBytes );
	if( data.recordedSamples == NULL )
	{
		printf("Could not allocate record array.\n");
		exit(1);
	}
	for( i=0; i<numSamples; i++ ) data.recordedSamples[i] = 0;

	err = Pa_Initialize();
	if( err != paNoError ) goto error;       
	 
/* Record some audio. -------------------------------------------- */
	err = Pa_OpenStream(
				&stream,
				Pa_GetDefaultInputDeviceID(),
				data.samplesPerFrame,               /* stereo input */
				PA_SAMPLE_TYPE,
				NULL,
				paNoDevice,
				0,
				PA_SAMPLE_TYPE,
				NULL,
				SAMPLE_RATE,
				1024,            /* frames per buffer */
				0,               /* number of buffers, if zero then use default minimum */
				paClipOff,  /* we won't output out of range samples so don't bother clipping them */
				recordCallback,
				&data );
	if( err != paNoError ) goto error;

	err = Pa_StartStream( stream );
	if( err != paNoError ) goto error;
	printf("Now recording!!\n"); fflush(stdout);

	while( Pa_StreamActive( stream ) )
	{
		Pa_Sleep(1000);
		printf("index = %d\n", data.frameIndex ); fflush(stdout);
	}

	err = Pa_CloseStream( stream );
	if( err != paNoError ) goto error;

/* Measure maximum peak amplitude. */
	max = 0;
	average = 0;
	for( i=0; i<numSamples; i++ )
	{
		val = data.recordedSamples[i];
		if( val < 0 ) val = -val; /* ABS */
		if( val > max )
		{
			max = val;
		}
		average += val;
	}
	printf("sample max amplitude = %d\n", max );
	average = average / numSamples;
	printf("sample average = %d\n", average );

/* Write recorded data to a file. */
#if 0
	{
		FILE  *fid;
		fid = fopen("recorded.raw", "wb");
		if( fid == NULL )
		{
			printf("Could not open file.");
		}
		else
		{
			fwrite( data.recordedSamples, data.samplesPerFrame * sizeof(SAMPLE), totalFrames, fid );
			fclose( fid );
			printf("Wrote data to 'recorded.raw'\n");
		}
	}
#endif

/* Playback recorded data.  -------------------------------------------- */
	data.frameIndex = 0;
	printf("Begin playback.\n"); fflush(stdout);
	err = Pa_OpenStream(
				&stream,
				paNoDevice,
				0,               /* NO input */
				PA_SAMPLE_TYPE,	
				NULL,
				Pa_GetDefaultOutputDeviceID(),
				data.samplesPerFrame,               /* stereo output */
				PA_SAMPLE_TYPE, 
				NULL,
				SAMPLE_RATE,
				1024,            /* frames per buffer */
				0,               /* number of buffers, if zero then use default minimum */
				paClipOff,       /* we won't output out of range samples so don't bother clipping them */
				playCallback,
				&data );
	if( err != paNoError ) goto error;

	if( stream )
	{
		err = Pa_StartStream( stream );
		if( err != paNoError ) goto error;
		printf("Waiting for playback to finish.\n"); fflush(stdout);

		while( Pa_StreamActive( stream ) ) Pa_Sleep(100);

		err = Pa_CloseStream( stream );
		if( err != paNoError ) goto error;
		printf("Done.\n"); fflush(stdout);
	}
	free( data.recordedSamples );

	Pa_Terminate();
	return 0;

error:
	Pa_Terminate();
	fprintf( stderr, "An error occured while using the portaudio stream\n" ); 
	fprintf( stderr, "Error number: %d\n", err );
	fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
	return -1;
}
