#ifndef PARTICLESYSTEM_HPP
#define PARTICLESYSTEM_HPP

#include "particlesystem.h"

#define MAX_THREAD 1024
#define MAX_BLOCK_X 65535ll
#define MAX_BLOCK_Y 65535ll
#define MAX_BLOCK_Z 65535ll


template <typename ParticleType>
ParticleSystem<ParticleType>::ParticleSystem(int const &n, Shader const &shader)
: nParticle(n),
  shader(shader){
	createVBO();
	setCallBacks();
	initCuda();
}

template <typename ParticleType>
ParticleSystem<ParticleType>::~ParticleSystem(){
	//unmap resource
	CUDA_SAFE_CALL( cudaGraphicsUnmapResources(1, &resource) );
	CUDA_SAFE_CALL( cudaGraphicsUnregisterResource(resource) );

	//free
	CUDA_SAFE_CALL( cudaFree(deviceParticles) );
}

template <typename ParticleType>
void ParticleSystem<ParticleType>::createVBO(){
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	//set VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, nParticle*sizeof(ParticleType), deviceParticles, GL_STATIC_DRAW);

	//set VAO
	//ParticleType::setVAO();
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ParticleType), (void*)(0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ParticleType), (void*)(sizeof(vec2)*1));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleType), (void*)(sizeof(vec2)*2));

	//unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

template <typename ParticleType>
void ParticleSystem<ParticleType>::setCallBacks() const{
	//glfwSetCursorPosCallback(scene.window, [](GLFWWindow *window, float x, float y){});
}


template <typename ParticleType>
void ParticleSystem<ParticleType>::initCuda(){
	deployGrid();

	// cuda allocations
	CUDA_SAFE_CALL( cudaMalloc((void**)&deviceParticles, nParticle*sizeof(ParticleType)) );

	//register buffer to cuda
	CUDA_SAFE_CALL( cudaGraphicsGLRegisterBuffer(&resource, VBO, cudaGraphicsRegisterFlagsNone) );

	//map dptr to VBO
	size_t retSz;
	ParticleType* dp;
	CUDA_SAFE_CALL( cudaGraphicsMapResources(1, &resource) );
	CUDA_SAFE_CALL( cudaGraphicsResourceGetMappedPointer((void**)&dp, &retSz, resource) );
}

template <typename ParticleType>
void ParticleSystem<ParticleType>::render(Mouse const &mouse){
	//set mouse position to device
	Mouse* deviceMouse = nullptr;
	auto sz = sizeof(Mouse);
	CUDA_SAFE_CALL( cudaMalloc((void**)&deviceMouse, sz) );
	CUDA_SAFE_CALL( cudaMemcpy(deviceMouse, &mouse, sz, cudaMemcpyHostToDevice) );

	//map dptr to VBO
	size_t retSz;
	ParticleType *dp = nullptr;
	CUDA_SAFE_CALL( cudaGraphicsResourceGetMappedPointer((void**)&dp, &retSz, resource) );
	//run cuda kernel
	updateKernel<<<block, grid>>>(dp, nParticle, *deviceMouse);
	CUDA_ERROR_CHECKER;
	CUDA_SAFE_CALL( cudaDeviceSynchronize() );

	//draw
	shader.use();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	glBindVertexArray(VAO);
	glDrawArrays(GL_POINTS, 0, nParticle);
	glBindVertexArray(0);

	glDisable(GL_BLEND);

	//free
	CUDA_SAFE_CALL( cudaFree(deviceMouse) );
}


template <typename ParticleType>
void ParticleSystem<ParticleType>::deployGrid(){
	unsigned int blockX = nParticle>MAX_THREAD? MAX_THREAD: static_cast<unsigned int>(nParticle);
	block = {blockX, 1, 1};

	float nGrid = static_cast<float>(nParticle)/blockX;
	if(nGrid > MAX_BLOCK_X*MAX_BLOCK_Y*MAX_BLOCK_Z)
		throw std::runtime_error("Number of particles out of gpu limits.");
	else if(nGrid > MAX_BLOCK_X*MAX_BLOCK_Y){
		unsigned int z = std::ceil(nGrid/MAX_BLOCK_X/MAX_BLOCK_Y);
		grid = {MAX_BLOCK_X, MAX_BLOCK_Y, z};
	}
	else if(nGrid > MAX_BLOCK_X){
		unsigned int y = std::ceil(nGrid/MAX_BLOCK_X);
		grid = {MAX_BLOCK_X, y, 1};
	}
	else if(nGrid > 0){
		unsigned int x = std::ceil(nGrid);
		grid = {x, 1, 1};
	}
	else
		throw std::runtime_error("No particles in screen.");
}

#endif