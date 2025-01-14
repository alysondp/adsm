/* ============================================================

Copyright (c) 2009 Advanced Micro Devices, Inc.  All rights reserved.

Redistribution and use of this material is permitted under the following
conditions:

Redistributions must retain the above copyright notice and all terms of this
license.

In no event shall anyone redistributing or accessing or using this material
commence or participate in any arbitration or legal action relating to this
material against Advanced Micro Devices, Inc. or any copyright holders or
contributors. The foregoing shall survive any expiration or termination of
this license or any agreement or access or use related to this material.

ANY BREACH OF ANY TERM OF THIS LICENSE SHALL RESULT IN THE IMMEDIATE REVOCATION
OF ALL RIGHTS TO REDISTRIBUTE, ACCESS OR USE THIS MATERIAL.

THIS MATERIAL IS PROVIDED BY ADVANCED MICRO DEVICES, INC. AND ANY COPYRIGHT
HOLDERS AND CONTRIBUTORS "AS IS" IN ITS CURRENT CONDITION AND WITHOUT ANY
REPRESENTATIONS, GUARANTEE, OR WARRANTY OF ANY KIND OR IN ANY WAY RELATED TO
SUPPORT, INDEMNITY, ERROR FREE OR UNINTERRUPTED OPERA TION, OR THAT IT IS FREE
FROM DEFECTS OR VIRUSES.  ALL OBLIGATIONS ARE HEREBY DISCLAIMED - WHETHER
EXPRESS, IMPLIED, OR STATUTORY - INCLUDING, BUT NOT LIMITED TO, ANY IMPLIED
WARRANTIES OF TITLE, MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
ACCURACY, COMPLETENESS, OPERABILITY, QUALITY OF SERVICE, OR NON-INFRINGEMENT.
IN NO EVENT SHALL ADVANCED MICRO DEVICES, INC. OR ANY COPYRIGHT HOLDERS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, PUNITIVE,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, REVENUE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED OR BASED ON ANY THEORY OF LIABILITY
ARISING IN ANY WAY RELATED TO THIS MATERIAL, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE. THE ENTIRE AND AGGREGATE LIABILITY OF ADVANCED MICRO DEVICES,
INC. AND ANY COPYRIGHT HOLDERS AND CONTRIBUTORS SHALL NOT EXCEED TEN DOLLARS
(US $10.00). ANYONE REDISTRIBUTING OR ACCESSING OR USING THIS MATERIAL ACCEPTS
THIS ALLOCATION OF RISK AND AGREES TO RELEASE ADVANCED MICRO DEVICES, INC. AND
ANY COPYRIGHT HOLDERS AND CONTRIBUTORS FROM ANY AND ALL LIABILITIES,
OBLIGATIONS, CLAIMS, OR DEMANDS IN EXCESS OF TEN DOLLARS (US $10.00). THE
FOREGOING ARE ESSENTIAL TERMS OF THIS LICENSE AND, IF ANY OF THESE TERMS ARE
CONSTRUED AS UNENFORCEABLE, FAIL IN ESSENTIAL PURPOSE, OR BECOME VOID OR
DETRIMENTAL TO ADVANCED MICRO DEVICES, INC. OR ANY COPYRIGHT HOLDERS OR
CONTRIBUTORS FOR ANY REASON, THEN ALL RIGHTS TO REDISTRIBUTE, ACCESS OR USE
THIS MATERIAL SHALL TERMINATE IMMEDIATELY. MOREOVER, THE FOREGOING SHALL
SURVIVE ANY EXPIRATION OR TERMINATION OF THIS LICENSE OR ANY AGREEMENT OR
ACCESS OR USE RELATED TO THIS MATERIAL.

NOTICE IS HEREBY PROVIDED, AND BY REDISTRIBUTING OR ACCESSING OR USING THIS
MATERIAL SUCH NOTICE IS ACKNOWLEDGED, THAT THIS MATERIAL MAY BE SUBJECT TO
RESTRICTIONS UNDER THE LAWS AND REGULATIONS OF THE UNITED STATES OR OTHER
COUNTRIES, WHICH INCLUDE BUT ARE NOT LIMITED TO, U.S. EXPORT CONTROL LAWS SUCH
AS THE EXPORT ADMINISTRATION REGULATIONS AND NATIONAL SECURITY CONTROLS AS
DEFINED THEREUNDER, AS WELL AS STATE DEPARTMENT CONTROLS UNDER THE U.S.
MUNITIONS LIST. THIS MATERIAL MAY NOT BE USED, RELEASED, TRANSFERRED, IMPORTED,
EXPORTED AND/OR RE-EXPORTED IN ANY MANNER PROHIBITED UNDER ANY APPLICABLE LAWS,
INCLUDING U.S. EXPORT CONTROL LAWS REGARDING SPECIFICALLY DESIGNATED PERSONS,
COUNTRIES AND NATIONALS OF COUNTRIES SUBJECT TO NATIONAL SECURITY CONTROLS.
MOREOVER, THE FOREGOING SHALL SURVIVE ANY EXPIRATION OR TERMINATION OF ANY
LICENSE OR AGREEMENT OR ACCESS OR USE RELATED TO THIS MATERIAL.

NOTICE REGARDING THE U.S. GOVERNMENT AND DOD AGENCIES: This material is
provided with "RESTRICTED RIGHTS" and/or "LIMITED RIGHTS" as applicable to
computer software and technical data, respectively. Use, duplication,
distribution or disclosure by the U.S. Government and/or DOD agencies is
subject to the full extent of restrictions in all applicable regulations,
including those found at FAR52.227 and DFARS252.227 et seq. and any successor
regulations thereof. Use of this material by the U.S. Government and/or DOD
agencies is acknowledgment of the proprietary rights of any copyright holders
and contributors, including those of Advanced Micro Devices, Inc., as well as
the provisions of FAR52.227-14 through 23 regarding privately developed and/or
commercial computer software.

This license forms the entire agreement regarding the subject matter hereof and
supersedes all proposals and prior discussions and writings between the parties
with respect thereto. This license does not affect any ownership, rights, title,
or interest in, or relating to, this material. No terms of this license can be
modified or waived, and no breach of this license can be excused, unless done
so in a writing signed by all affected parties. Each term of this license is
separately enforceable. If any term of this license is determined to be or
becomes unenforceable or illegal, such term shall be reformed to the minimum
extent necessary in order for this license to remain in effect in accordance
with its terms as modified by such reformation. This license shall be governed
by and construed in accordance with the laws of the State of Texas without
regard to rules on conflicts of law of any state or jurisdiction or the United
Nations Convention on the International Sale of Goods. All disputes arising out
of this license shall be subject to the jurisdiction of the federal and state
courts in Austin, Texas, and all defenses are hereby waived concerning personal
jurisdiction and venue of these courts.

============================================================ */


#include<GL/glut.h>
#include <cmath>
#include<malloc.h>

#include <fstream>
#include <iostream>

#include "NBody.h"

#include "utils.h"

int numBodies;      /**< No. of particles*/
cl_float* pos;      /**< Output position */
void* me;           /**< Pointing to NBody class */
cl_bool display;

clock_t t1, t2;
int frameCount = 0;
int frameRefCount = 90;
double totalElapsedTime = 0.0;

bool quiet = false;
bool verify = false;
bool timing = true;

float
NBody::random(float randMax, float randMin)
{
    float result;
    result =(float)rand()/(float)RAND_MAX;

    return ((1.0f - result) * randMin + result *randMax);
}

int
NBody::setupNBody()
{
    // make sure numParticles is multiple of group size
    numBodies = numParticles;

    initPos = (cl_float*)malloc(numBodies * sizeof(cl_float4));
    assert(initPos != NULL);
    initVel = (cl_float*)malloc(numBodies * sizeof(cl_float4));
    assert(initVel != NULL);
#if defined (_WIN32)
    pos = (cl_float*)_aligned_malloc(numBodies * sizeof(cl_float4), 16);
#else
    pos = (cl_float*)memalign(16, numBodies * sizeof(cl_float4));
#endif
    assert(pos != NULL);
#if defined (_WIN32)
    vel = (cl_float*)_aligned_malloc(numBodies * sizeof(cl_float4), 16);
#else
    vel = (cl_float*)memalign(16, numBodies * sizeof(cl_float4));
#endif
    assert(vel != NULL);
    /* initialization of inputs */
    for(int i = 0; i < numBodies; ++i)
    {
        int index = 4 * i;

        // First 3 values are position in x,y and z direction
        for(int j = 0; j < 3; ++j)
        {
            initPos[index + j] = random(3, 50);
        }

        // Mass value
        initPos[index + 3] = random(1, 1000);

        // First 3 values are velocity in x,y and z direction
        for(int j = 0; j < 3; ++j)
        {
            initVel[index + j] = 0.0f;
        }

        // unused
        initVel[3] = 0.0f;
    }

    memcpy(pos, initPos, 4 * numBodies * sizeof(cl_float));
    memcpy(vel, initVel, 4 * numBodies * sizeof(cl_float));

    return 0;
}

int
NBody::setupCL()
{
    cl_int status = CL_SUCCESS;

    cl_uint numPlatforms;
    cl_platform_id platform = NULL;
    assert(clGetPlatformIDs(0, NULL, &numPlatforms) == CL_SUCCESS);
    assert(numPlatforms > 0);
    cl_platform_id* platforms = new cl_platform_id[numPlatforms];
    assert(clGetPlatformIDs(numPlatforms, platforms, NULL) == CL_SUCCESS);
    platform = platforms[0];
    delete[] platforms;

    cl_context_properties cps[3] = 
    {
        CL_CONTEXT_PLATFORM, 
        (cl_context_properties)platform, 
        0
    };

    context = clCreateContextFromType(cps, CL_DEVICE_TYPE_GPU, NULL, NULL, &status);
    assert(status == CL_SUCCESS);

    size_t deviceListSize;
    /* First, get the size of device list data */
    assert(clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &deviceListSize) == 0);
    int deviceCount = (int)(deviceListSize / sizeof(cl_device_id));
    assert(deviceCount > 0);

    devices = (cl_device_id*)malloc(deviceListSize);
    assert(clGetContextInfo(context, CL_CONTEXT_DEVICES, deviceListSize, devices, NULL) == 0);

    commandQueue = clCreateCommandQueue(context, devices[0], 0, &status);
    assert(status == CL_SUCCESS);

    /*
    * Create and initialize memory objects
    */

    /* Create memory objects for position */
    currPos = clCreateBuffer(context, CL_MEM_READ_WRITE,
                             numBodies * sizeof(cl_float4), 0, &status);
    assert(status == CL_SUCCESS);

    /* Initialize position buffer */
    status = clEnqueueWriteBuffer(commandQueue, currPos, 1, 0,
                                  numBodies * sizeof(cl_float4), pos,
                                  0, 0, 0);
    assert(status == CL_SUCCESS);

    /* Create memory objects for position */
    newPos = clCreateBuffer(context, CL_MEM_READ_WRITE,
        numBodies * sizeof(cl_float4), 0, &status);
    assert(status == CL_SUCCESS);
    /* Create memory objects for velocity */
    currVel = clCreateBuffer(context, CL_MEM_READ_WRITE,
        numBodies * sizeof(cl_float4), 0, &status);
    assert(status == CL_SUCCESS);

    /* Initialize velocity buffer */
    status = clEnqueueWriteBuffer(commandQueue, currVel, 1, 0,
                                  numBodies * sizeof(cl_float4),
                                  vel, 0, 0, 0);
    assert(status == CL_SUCCESS);

    /* Create memory objects for velocity */
    newVel = clCreateBuffer(context, CL_MEM_READ_ONLY,
        numBodies * sizeof(cl_float4), 0, &status);
    assert(status == CL_SUCCESS);

    std::ifstream in("NBody_Kernels.cl", std::ios_base::in);
    if (!in.good()) {
        std::cout << "Failed to load kernel file : " << "NBody_Kernels.cl" << std::endl;
        abort();
    }
    in.seekg(0, std::ios::end);
    size_t length = size_t(in.tellg());
    in.seekg(0, std::ios::beg);
    // Allocate memory for the code
    char *source = new char[length+1];
    // Read data as a block
    in.read(source,length);
    source[length] = '\0';
    in.close();

    const char *constSrc = source;

    program = clCreateProgramWithSource(context, 1, &constSrc, &length, &status);
    assert(status == CL_SUCCESS);

    // Get additional options
    /* create a cl program executable for all the devices specified */
    assert(clBuildProgram(program, 1, devices, NULL, NULL, NULL) ==
           CL_SUCCESS);
    
    /* get a kernel object handle for a kernel with the given name */
    kernel = clCreateKernel(program, "nbody_sim", &status);
    assert(status == CL_SUCCESS);

    delete [] source;

    return 0;
}


int
NBody::setupCLKernels()
{
    /* Set appropriate arguments to the kernel */

    /* Particle positions */
    assert(clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&currPos) == CL_SUCCESS);
    /* Particle velocity */
    assert(clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&currVel) == CL_SUCCESS);
    /* numBodies */
    assert(clSetKernelArg(kernel, 2, sizeof(cl_int), (void *)&numBodies) == CL_SUCCESS);
    /* time step */
    assert(clSetKernelArg(kernel, 3, sizeof(cl_float), (void *)&delT) == CL_SUCCESS);
    /* upward Pseudoprobability */
    assert(clSetKernelArg(kernel, 4, sizeof(cl_float), (void *)&espSqr) == CL_SUCCESS);
    /* local memory */
    assert(clSetKernelArg(kernel, 5, GROUP_SIZE * 4 * sizeof(float), NULL) == CL_SUCCESS);
    /* Particle positions */
    assert(clSetKernelArg(kernel, 6, sizeof(cl_mem), (void*)&newPos) == CL_SUCCESS);
    /* Particle velocity */
    assert(clSetKernelArg(kernel, 7, sizeof(cl_mem), (void *)&newVel) == CL_SUCCESS);

    return 0;
}

int
NBody::runCLKernels()
{
    cl_int status;
    cl_event events[1];

    /*
    * Enqueue a kernel run call.
    */
    size_t globalThreads[] = {numBodies};
    size_t localThreads[] = {256};

    assert(clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL,
        globalThreads, localThreads, 0, NULL, NULL) == CL_SUCCESS);

    assert(clFinish(commandQueue) == CL_SUCCESS);

    assert(clEnqueueCopyBuffer(commandQueue, newPos, currPos, 0, 0,
        sizeof(cl_float4) * numBodies, 0, 0, 0) == CL_SUCCESS);

    assert(clEnqueueCopyBuffer(commandQueue, newVel, currVel, 0, 0,
        sizeof(cl_float4) * numBodies, 0, 0, 0) == CL_SUCCESS);

    assert(clFinish(commandQueue) == CL_SUCCESS);
    /* Enqueue readBuffer*/
    assert(clEnqueueReadBuffer(commandQueue, currPos, CL_TRUE, 0,
        numBodies* sizeof(cl_float4), pos, 0, NULL, &events[0]) == CL_SUCCESS);
    /* Wait for the read buffer to finish execution */
    assert(clWaitForEvents(1, &events[0]) == CL_SUCCESS);

    clReleaseEvent(events[0]);

    return 0;
}

/*
* n-body simulation on cpu
*/
void
NBody::nBodyCPUReference()
{
    //Iterate for all samples
    for(int i = 0; i < numBodies; ++i) {
        int myIndex = 4 * i;
        float acc[3] = {0.0f, 0.0f, 0.0f};
        for(int j = 0; j < numBodies; ++j) {
            float r[3];
            int index = 4 * j;

            float distSqr = 0.0f;
            for(int k = 0; k < 3; ++k) {
                r[k] = refPos[index + k] - refPos[myIndex + k];

                distSqr += r[k] * r[k];
            }

            float invDist = 1.0f / sqrt(distSqr + espSqr);
            float invDistCube =  invDist * invDist * invDist;
            float s = refPos[index + 3] * invDistCube;

            for(int k = 0; k < 3; ++k)
            {
                acc[k] += s * r[k];
            }
        }

        for(int k = 0; k < 3; ++k)
        {
            refPos[myIndex + k] += refVel[myIndex + k] * delT + 0.5f * acc[k] * delT * delT;
            refVel[myIndex + k] += acc[k] * delT;
        }
    }
}

int
NBody::setup()
{
    setupNBody();

    setupCL();

    display = !quiet && !verify;

    return 0;
}

/**
* @brief Initialize GL
*/
void
GLInit()
{
    glClearColor(0.0 ,0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
}

/**
* @brief Glut Idle function
*/
void
idle()
{
    glutPostRedisplay();
}

/**
* @brief Glut reshape func
*
* @param w numParticles of OpenGL window
* @param h height of OpenGL window
*/
void
reShape(int w,int h)
{
    glViewport(0, 0, w, h);

    glViewport(0, 0, w, h);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluPerspective(45.0f, w/h, 1.0f, 1000.0f);
    gluLookAt (0.0, 0.0, -2.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0);
}

/**
* @brief OpenGL display function
*/
void displayfunc()
{
    gmactime_t s, t;
    getTime(&s);
    frameCount++;

    glClearColor(0.0 ,0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);

    glPointSize(1.0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);

    glColor3f(1.0f,0.6f,0.0f);

    //Calling kernel for calculatig subsequent positions
    ((NBody*)me)->runCLKernels();

    glBegin(GL_POINTS);
    for(int i=0; i < numBodies; ++i)
    {
        //divided by 300 just for scaling
        glVertex3d(pos[i*4+ 0]/300, pos[i*4+1]/300, pos[i*4+2]/300);
    }
    glEnd();

    glFlush();
    glutSwapBuffers();

    getTime(&t);
    totalElapsedTime += (getTimeStamp(t) - getTimeStamp(s)); //(double)(t2 - t1);
    if(frameCount > frameRefCount) {
        // set GLUT Window Title
        char title[256];
        int framesPerSec = (int)(frameCount / (totalElapsedTime / 1e6));
#if defined (_WIN32) && !defined(__MINGW32__)
        sprintf_s(title, 256, "OpenCL NBody | %d fps ", framesPerSec);
#else 
        sprintf(title, "OpenCL NBody | %d fps", framesPerSec);
#endif
        glutSetWindowTitle(title);
        frameCount = 0;
        totalElapsedTime = 0.0;
    }
}

/* keyboard function */
void
keyboardFunc(unsigned char key, int mouseX, int mouseY)
{
    switch(key)
    {
        /* If the user hits escape or Q, then exit */
        /* ESCAPE_KEY = 27 */
    case 27:
    case 'q':
    case 'Q':
        {
                exit(0);
        }
    default:
        break;
    }
}


int
NBody::run()
{
    /* Arguments are set and execution call is enqueued on command buffer */
    setupCLKernels();

    if(verify || timing) {
        for(int i = 0; i < iterations; ++i) {
            runCLKernels();
        }
    }

    return 0;
}

static bool
compareVector(cl_float *a, cl_float *b, unsigned len, cl_float threshold)
{
    for (unsigned i = 0; i < len; i++) {
        if (fabsf(a[i] - b[i]) >= threshold) return false;
    }

    return true;
}

int
NBody::verifyResults()
{
    if(verify) {
        /* reference implementation
        * it overwrites the input array with the output
        */
        refPos = (cl_float*)malloc(numBodies * sizeof(cl_float4));
        assert(refPos != NULL);
        refVel = (cl_float*)malloc(numBodies * sizeof(cl_float4));
        assert(refVel != NULL);

        memcpy(refPos, initPos, 4 * numBodies * sizeof(cl_float));
        memcpy(refVel, initVel, 4 * numBodies * sizeof(cl_float));

        for(int i = 0; i < iterations; ++i) {
            nBodyCPUReference();
        }

        /* compare the results and see if they match */
        if(!compareVector(pos, refPos, 4 * numBodies, 0.00001)) {
            exit(1);
        }
    }

    return 0;
}

void
NBody::printStats()
{
    // TODO Implement timing
#if 0
    std::string strArray[4] =
    {
        "Particles",
        "Iterations",
        "Time(sec)",
        "kernelTime(sec)"
    };

    std::string stats[4];
    totalTime = setupTime + kernelTime;

    stats[0] = sampleCommon->toString(numParticles, std::dec);
    stats[1] = sampleCommon->toString(iterations, std::dec);
    stats[2] = sampleCommon->toString(totalTime, std::dec);
    stats[3] = sampleCommon->toString(kernelTime, std::dec);

    this->SDKSample::printStats(strArray, stats, 4);
#endif
}

int
NBody::cleanup()
{
    /* Releases OpenCL resources (Context, Memory etc.) */
    assert(clReleaseKernel(kernel) == CL_SUCCESS);
    assert(clReleaseProgram(program) == CL_SUCCESS);
    assert(clReleaseMemObject(currPos) == CL_SUCCESS);
    assert(clReleaseMemObject(currVel) == CL_SUCCESS);
    assert(clReleaseCommandQueue(commandQueue) == CL_SUCCESS);
    assert(clReleaseContext(context) == CL_SUCCESS);

    return 0;
}

NBody::~NBody()
{
    /* release program resources */
    if(initPos) {
        free(initPos);
        initPos = NULL;
    }

    if(initVel) {
        free(initVel);
        initVel = NULL;
    }

    if(pos) {
#if defined (_WIN32)
        _aligned_free(pos);
#else
        free(pos);
#endif
        pos = NULL;
    }
    if(vel) {
#if defined (_WIN32)
        _aligned_free(vel);
#else
        free(vel);
#endif
        vel = NULL;
    }

    if(devices)
    {
        free(devices);
        devices = NULL;
    }

    if(refPos) {
        free(refPos);
        refPos = NULL;
    }

    if(refVel) {
        free(refVel);
        refVel = NULL;
    }
}


int
main(int argc, char * argv[])
{
    NBody clNBody("OpenCL NBody");
    me = &clNBody;

    clNBody.setup();
    clNBody.run();
    clNBody.verifyResults();

    clNBody.printStats();

    if(display) {
        // Run in  graphical window if requested
        glutInit(&argc, argv);
        glutInitWindowPosition(100,10);
        glutInitWindowSize(600,600);
        glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE );
        glutCreateWindow("nbody simulation");
        GLInit();
        glutDisplayFunc(displayfunc);
        glutReshapeFunc(reShape);
        glutIdleFunc(idle);
        glutKeyboardFunc(keyboardFunc);
        glutMainLoop();
    }

    clNBody.cleanup();

    return 0;
}
