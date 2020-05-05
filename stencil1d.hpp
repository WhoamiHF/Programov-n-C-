#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <utility>
#include <algorithm>
#include <iostream>
#include<condition_variable>
template<typename ET>
class circle;

template<typename ET, typename SF>
class forwarder {
public:
	
	void operator()(void* context, typename std::vector<ET>::iterator beg, typename std::vector<ET>::iterator end, size_t G, SF& sf, size_t g, size_t index, size_t threads)
	{
		static_cast<circle<ET>*>(context)->oneThreadJob(beg, end, G, sf, g, index, threads);
	}
};


template<typename ET>
class circle
{
public:
	circle(std::size_t s) :field(std::vector<ET>(s)) {}

	std::size_t size() const 
	{
		return field.size();
	}
	//Computes real index by adding size() to parametr and then aplying mod size(),sets value of índexed element
	void set(std::ptrdiff_t x, const ET& v) 
	{
		std::ptrdiff_t index = (x + size()) % size();
		field[index] = v;
	}
	//Computes real index by adding size() to parametr and then aplying mod size(),gets value of índexed element
	decltype(auto) get(std::ptrdiff_t x) const 
	{
		std::ptrdiff_t index = (x + size()) % size();
		return field[index];
	}
	//this is the function which does each thread. First it sends messages to its neighbors, then recieves from them and then does G (or less) generations. 
	template<typename SF>
	void oneThreadJob(typename std::vector<ET>::iterator beg, typename std::vector<ET>::iterator end, size_t G, SF& sf, size_t g, size_t index, size_t threads)
	{

		threeVectors storage;
		storage.thisPart = std::vector(beg, end);

		//Creates second threeVectors, set them on correct sizes, allocates once
		//@todo constructor
		threeVectors secondaryStorage;
		secondaryStorage.leftG = std::vector<ET>(G);
		secondaryStorage.thisPart = std::vector<ET>(storage.thisPart.size());
		secondaryStorage.rightG = std::vector<ET>(G);
		for(size_t generationIndex = 1;generationIndex<=g;generationIndex+=G)
		{

			//sending
			{
				std::lock_guard<std::mutex> lock(mutexesToLeft[index]);
				channelsToLeft[index].push(std::vector<ET>(storage.thisPart.begin(), storage.thisPart.begin() + G));
			}
			cond_variables_to_left[index].notify_one();
			// uses index of thread, for each two neighbor threads there are two channels - toLeft and toRight
			{
				std::lock_guard<std::mutex> lock(mutexesToRight[index]);
				channelsToRight[index].push(std::vector<ET>(storage.thisPart.end() - G, storage.thisPart.end()));
			}
			cond_variables_to_right[index].notify_one();
			//recieving   @todo:notifications - not needed 
			//@todo make sure index are correct

			{
				size_t indexRight = (index + 1) % threads;
				std::unique_lock<std::mutex> lock(mutexesToLeft[indexRight]);
				while (channelsToLeft[indexRight].empty())
				{
					cond_variables_to_left[indexRight].wait(lock);
				}
					storage.rightG = channelsToLeft[indexRight].front();
					channelsToLeft[indexRight].pop();
			}
			{
				size_t indexLeft = (index - 1 + threads) % threads;
				std::unique_lock<std::mutex> lock2(mutexesToRight[indexLeft]);
				while (channelsToRight[indexLeft].empty())
				{
					cond_variables_to_right[indexLeft].wait(lock2);
				}
					storage.leftG = channelsToRight[indexLeft].front();
					channelsToRight[indexLeft].pop();
			}

			//swap pointers
			//computing G (or less) generations. In each generation it computes only valid part of the vector. 
			for (size_t i = 1; i <= G && i <= g; i++)
			{
				for (size_t j = i; j < storage.size() - i; j++)
				{
					secondaryStorage[j] = sf(storage[j - 1], storage[j], storage[j + 1]);
				}
				std::swap(secondaryStorage, storage);
			}
		}
		//move
		//@todo write back -> warning write with caution
		{
			std::unique_lock<std::mutex> lock(mutexToWriteData);
			for (size_t i = 0; i < storage.thisPart.size(); i++)
			{
				*(beg + i) = storage[i + storage.leftG.size()];
			}
		}
		//@todo clean (/ local in run) make sure not locked mutex
		//one thread,low amount of generations

	}
	//run divides the field to threads
	template<typename SF>
	void run(SF&& sf, std::size_t g, std::size_t thrs = std::thread::hardware_concurrency()) 
	{
		std::vector<std::thread> workers;
		size_t W = size() / thrs;
		size_t G = W/8;
		if (G < 1) { G += 1; }
		mutexesToLeft = std::vector<std::mutex>(thrs);
		mutexesToRight = std::vector<std::mutex>(thrs);
		cond_variables_to_left = std::vector<std::condition_variable>(thrs);
		cond_variables_to_right = std::vector<std::condition_variable>(thrs);
		channelsToLeft = std::vector<std::queue<std::vector<ET>>>(thrs);
		channelsToRight = std::vector<std::queue<std::vector<ET>>>(thrs);

		typename std::vector<ET>::iterator startingPosition = field.begin();
		for (size_t th = 0; th < thrs; th++) 
		{
			if (size() % thrs > th) 
			{
				workers.push_back(std::thread(forwarder<ET,SF>(),this,startingPosition, startingPosition + W + 1, G, std::ref(sf), g, th, thrs));
				startingPosition += (W + 1);
			}
			else
			{
				workers.push_back(std::thread(forwarder<ET, SF>(), this, startingPosition, startingPosition + W, G, std::ref(sf), g, th, thrs));
				startingPosition += W;
			}
		}
		for (auto& t : workers)
			t.join();
	}
	//each thread creates two of this object. This class contains three parts - leftG will be recieved from left, thisPart is computedby this thread
	//and rightG will be recieved from right neighbor. It can be indexed without knowing how the parts are divided.
	class threeVectors 
	{
	public:
		decltype(auto) operator[](size_t index) 
		{
			if (index < leftG.size()) 
			{
				return leftG[index];
			}
			else if (index < leftG.size() + thisPart.size())
			{
				return thisPart[index - leftG.size()];
			}
			else 
			{
				return rightG[index - leftG.size() - thisPart.size()];
			}

		}
		size_t size() 
		{
			return leftG.size() + thisPart.size() + rightG.size();
		}
		std::vector<ET> leftG;
		std::vector<ET> thisPart;
		std::vector<ET> rightG;
	};
	
private:
	std::vector<ET> field;
	std::vector<std::mutex> mutexesToRight;
	std::vector<std::mutex> mutexesToLeft;
	std::vector<std::condition_variable> cond_variables_to_right;
	std::vector<std::condition_variable> cond_variables_to_left;
	std::vector<std::queue<std::vector<ET>>> channelsToRight;
	std::vector<std::queue<std::vector<ET>>> channelsToLeft;
	std::mutex mutexToWriteData;
};


