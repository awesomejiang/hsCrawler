#include "gridcells.h"

GridCells::GridCells(float h): cellDim{static_cast<unsigned int>(1.0f/h)}, cellNum{cellDim*cellDim*cellDim} {
	//alloc memory for neighbor grids
	CUDA_SAFE_CALL( cudaMalloc((void**)&cells, sizeof(DeviceGridCell)*cellNum) );

	//init cells
	deployGrid();
	//initGridCells<<<grid, block>>>(cells, cellNum);
	//CUDA_ERROR_CHECKER;
}

GridCells::~GridCells(){
	CUDA_SAFE_CALL( cudaFree(cells) );
	//destroyGridCells<<<grid, block>>>(cells, cellNum);
	//CUDA_ERROR_CHECKER;
}

void GridCells::deployGrid(){
	//set block and grid
	//no communications even between threads, so configuration method is not important.
	unsigned int blockX = cellNum>MAX_THREAD? MAX_THREAD: cellNum;
	block = {blockX, 1, 1};

	float nGrid = static_cast<float>(cellNum)/blockX;
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

void GridCells::clear(){
	clearGridCells<<<grid, block>>>(cells, cellNum);
	CUDA_ERROR_CHECKER;
}

void GridCells::insertParticle(DeviceParticle const *p, dim3 g, dim3 b, int num){
	updateGridCells<<<g, b>>>(cells, p, num, cellDim);
	CUDA_ERROR_CHECKER;
}


DeviceGridCell * GridCells::getCells() const{
	return cells;
}


__GLOBAL__ void initGridCells(DeviceGridCell *cells, int cellNum){
	auto idx = getIdx();
	if(idx >= cellNum)
		return ;

	//cells[idx].pos = new vec3[MAX_PARTICLE];  
}


__GLOBAL__ void clearGridCells(DeviceGridCell *cells, int cellNum){
	auto idx = getIdx();
	if(idx >= cellNum)
		return ;

	cells[idx].num = 0;
}


__GLOBAL__ void updateGridCells(DeviceGridCell *cells, DeviceParticle const *p, int pNum, int cellDim){
	auto idx = getIdx();
	if(idx >= pNum)
		return ;

	auto pos = p[idx].pos;
	int iX = (pos[0]+1.0f)/2*cellDim;
	int iY = (pos[1]+1.0f)/2*cellDim;
	int iZ = (pos[2]+1.0f)/2*cellDim;
	auto iCell = iX*cellDim*cellDim + iY*cellDim + iZ;

	auto n = atomicAdd(&(cells[iCell].num), 1);
	if(n < MAX_PARTICLE)
		cells[iCell].pos[n] = pos;
	else
		atomicSub(&(cells[iCell].num), 1);
}


__GLOBAL__ void destroyGridCells(DeviceGridCell *cells, int cellNum){
	auto idx = getIdx();
	if(idx >= cellNum)
		return ;

	//delete[] cells[idx].pos;
}
