#include <cstdio>
#include <cstdlib>
#include <ctime>

#include "gmac/opencl.h"

#include "utils.h"
#include "debug.h"

#include "../eclBinomialOptionKernel.cl"

#define VOLATILITY 0.30f
#define RISKFREE 0.02f

int
binomialOptionCPUReference(cl_float *refOutput, cl_float *randArray, cl_int numSamples, cl_int numSteps)
{
	float* stepsArray = (float*)malloc((numSteps + 1) * sizeof(cl_float4));
	if(stepsArray== NULL)
		return 0;
	/* Iterate for all samples */
	for(int bid = 0; bid < numSamples; ++bid) {
		float s[4];
		float x[4];
		float vsdt[4];
		float puByr[4];
		float pdByr[4];
		float optionYears[4];

		float inRand[4];

		for(int i = 0; i < 4; ++i) {
			inRand[i] = randArray[bid + i];
			s[i] = (1.0f - inRand[i]) * 5.0f + inRand[i] * 30.f;
			x[i] = (1.0f - inRand[i]) * 1.0f + inRand[i] * 100.f;
			optionYears[i] = (1.0f - inRand[i]) * 0.25f + inRand[i] * 10.f;
			float dt = optionYears[i] * (1.0f / (float)numSteps);
			vsdt[i] = VOLATILITY * sqrtf(dt);
			float rdt = RISKFREE * dt;
			float r = expf(rdt);
			float rInv = 1.0f / r;
			float u = expf(vsdt[i]);
			float d = 1.0f / u;
			float pu = (r - d)/(u - d);
			float pd = 1.0f - pu;
			puByr[i] = pu * rInv;
			pdByr[i] = pd * rInv;
		}
		// Compute values at expiration date:
		// Call option value at period end is v(t) = s(t) - x
		// If s(t) is greater than x, or zero otherwise...
		// The computation is similar for put options...
		for(int j = 0; j <= numSteps; j++) {
			for(int i = 0; i < 4; ++i) {
				float profit = s[i] * expf(vsdt[i] * (2.0f * j - numSteps)) - x[i];
				stepsArray[j * 4 + i] = profit > 0.0f ? profit : 0.0f;
			}
		}

		//walk backwards up on the binomial tree of depth numSteps
		//Reduce the price step by step
		for(int j = numSteps; j > 0; --j) {
			for(int k = 0; k <= j - 1; ++k) {
				for(int i = 0; i < 4; ++i) {
					stepsArray[k * 4 + i] = pdByr[i] * stepsArray[(k + 1) * 4 + i] + puByr[i] * stepsArray[k * 4 + i];
				}
			}
		}

		//Copy the root to result
		refOutput[bid] = stepsArray[0];
	}

	free(stepsArray);

	return 0;
}

int main(int argc, char *argv[])
{
	gmactime_t s, t, S, T;

	cl_float* randArray = NULL;
	cl_float* output = NULL;
	cl_float* refOutput;
	cl_int numSamples = 64;
	cl_int numSteps = 254;

	assert(eclCompileSource(code) == eclSuccess);

	getTime(&s);
	// Alloc & init data
	assert(eclMalloc((void **)&randArray, numSamples * sizeof(cl_float4)) == eclSuccess);
	assert(eclMalloc((void **)&output, numSamples * sizeof(cl_float4)) == eclSuccess);
	refOutput = (float*)malloc(numSamples * sizeof(cl_float4));
	if(refOutput == NULL)
		return 0;
	getTime(&t);
	printTime(&s, &t, "Alloc: ", "\n");

	getTime(&S);
	getTime(&s);
	/* random initialisation of input */
	for(int i = 0; i < numSamples * 4; i++) {
		randArray[i] = (float)rand() / (float)RAND_MAX;
	}
	valueInit(output, 0, numSamples * 4);
	getTime(&t);
	printTime(&s, &t, "Init: ", "\n");

	getTime(&s);
	ecl_kernel kernel;
	assert(eclGetKernel("binomial_options", &kernel) == eclSuccess);
	assert(eclSetKernelArg(kernel, 0, sizeof(numSteps), &numSteps) == eclSuccess);
	assert(eclSetKernelArgPtr(kernel, 1, randArray) == eclSuccess);
	assert(eclSetKernelArgPtr(kernel, 2, output) == eclSuccess);
	assert(eclSetKernelArg(kernel, 3, (numSteps + 1) * sizeof(cl_float4), NULL) == eclSuccess);
	assert(eclSetKernelArg(kernel, 4, numSteps * sizeof(cl_float4), NULL) == eclSuccess);

	size_t globalThreads[] = {numSamples * (numSteps + 1)};
	size_t localThreads[] = {numSteps + 1};
	assert(eclCallNDRange(kernel, 1, NULL, globalThreads, localThreads) == eclSuccess);
	getTime(&t);
	printTime(&s, &t, "Run: ", "\n");

	printf("Output: ");
	for(int i = 0; i < numSamples; i++) {
		printf("%f ", output[i]);
	}

	getTime(&s);
	bool result = 1;
	binomialOptionCPUReference(refOutput, randArray, numSamples, numSteps);
	float error = 0.0f;
	float ref = 0.0f;

	for(int i = 1; i < numSamples; ++i) {
		float diff = output[i] - refOutput[i];
		error += diff * diff;
		ref += output[i] * output[i];
	}

	float normRef =::sqrtf((float) ref);
	if (::fabs((float) ref) < 1e-7f) {
		result = 0;
	}
	if(result) {
		float normError = ::sqrtf((float) error);
		error = normError / normRef;
		result = error < 0.001f;
	}
	if(result)
		printf("\nPassed!\n");
	else
		printf("\nFailed!\n");
	getTime(&t);
	printTime(&s, &t, "Check: ", "\n");
	getTime(&T);
	printTime(&S, &T, "Total: ", "\n");

	getTime(&s);
	free(refOutput);
	refOutput = NULL;
	eclReleaseKernel(kernel);
	eclFree(randArray);
	eclFree(output);
	getTime(&t);
	printTime(&s, &t, "Free: ", "\n");
	return 0;
}

