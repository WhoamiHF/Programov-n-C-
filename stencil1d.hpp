#pragma once
#include <vector>
#include <queue>

#include <thread>
#include <mutex>
#include<condition_variable>

template<typename ET>
class circle;

//Contains three components for exchange channels between threads. 
template<typename ET>
class package
{
public:
	std::mutex mtx;
	std::condition_variable condition;
	std::queue<std::vector<ET>> channel;
};

//Functor which enables calling member function I had some errors with direct calling (member function)
template<typename ET, typename SF>
class forwarder
{
public:
	void operator()(void* context, typename std::vector<ET>::iterator beg, typename std::vector<ET>::iterator end, size_t G, SF& sf, size_t g, size_t index,
		size_t threads, typename std::vector<ET>::iterator res
		, package<ET>& toLeft, package<ET>& toRight, package<ET>& fromLeft, package<ET>& fromRight, std::mutex& mutexToWriteData)
	{
		static_cast<circle<ET>*>(context)->oneThreadComputing(beg, end, G, sf, g, index, threads, res, toLeft, toRight, fromLeft, fromRight, mutexToWriteData);
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

	//run divides the field to threads and calls oneThreadComputing on each of them
	template<typename SF>
	void run(SF&& sf, std::size_t g, std::size_t thrs = std::thread::hardware_concurrency())
	{
		std::vector<package<ET>> toLeft(thrs);
		std::vector<package<ET>> toRight(thrs);

		std::mutex mutexToWriteData;
		std::vector<ET> result(size());

		typename std::vector<ET>::iterator startOfResult = result.begin();
		typename std::vector<ET>::iterator startingPosition = field.begin();

		std::vector<std::thread> workers;
		size_t W = size() / thrs;
		size_t G = W / 32;
		if (G == 0) { G = 1; }

		for (size_t th = 0; th < thrs; th++)
		{
			if (size() % thrs > th)
			{
				workers.push_back(std::thread(forwarder<ET, SF>(), this, startingPosition, startingPosition + W + 1, G, std::ref(sf), g, th, thrs, startOfResult
					, std::ref(toLeft[th]), std::ref(toRight[th]), std::ref(toRight[(th - 1 + thrs) % thrs]), std::ref(toLeft[(th + 1) % thrs]), std::ref(mutexToWriteData)
				));
				startingPosition += (W + 1);
				startOfResult += (W + 1);
			}
			else
			{
				workers.push_back(std::thread(forwarder<ET, SF>(), this, startingPosition, startingPosition + W, G, std::ref(sf), g, th, thrs, startOfResult
					, std::ref(toLeft[th]), std::ref(toRight[th]), std::ref(toRight[(th - 1 + thrs) % thrs]), std::ref(toLeft[(th + 1) % thrs]), std::ref(mutexToWriteData)
				));
				startingPosition += W;
				startOfResult += W;
			}
		}
		for (auto& t : workers)
		{
			t.join();
		}
		field = std::move(result);
	}
	//this is the function which is done by each thread. First it sends messages to its neighbors, then recieves from them and then does G (or less) generations. 
	template<typename SF>
	void oneThreadComputing(typename std::vector<ET>::iterator beg, typename std::vector<ET>::iterator end, size_t generationsBlock, SF& sf, size_t generationsTotal, size_t index, size_t threads,
		typename std::vector<ET>::iterator res, package<ET>& toLeft, package<ET>& toRight, package<ET>& fromLeft, package<ET>& fromRight, std::mutex& mutexToWriteData)
	{
		threeVectors storage;
		storage.thisPart = std::vector(beg, end);

		//Creates second threeVectors, set them on correct sizes, allocates once
		threeVectors secondaryStorage;
		secondaryStorage.leftG = std::vector<ET>(generationsBlock);
		secondaryStorage.thisPart = std::vector<ET>(storage.thisPart.size());
		secondaryStorage.rightG = std::vector<ET>(generationsBlock);
		for (size_t generationIndex = 0; generationIndex < generationsTotal; generationIndex += generationsBlock)
		{

			//sending
			{
				std::lock_guard<std::mutex> lock(toLeft.mtx);
				toLeft.channel.push(std::vector<ET>(storage.thisPart.begin(), storage.thisPart.begin() + generationsBlock));
			}
			toLeft.condition.notify_one();

			{
				std::lock_guard<std::mutex> lock(toRight.mtx);
				toRight.channel.push(std::vector<ET>(storage.thisPart.end() - generationsBlock, storage.thisPart.end()));
			}
			toRight.condition.notify_one();
			//recieving
			{
				std::unique_lock<std::mutex> lock(fromRight.mtx);
				while (fromRight.channel.empty())
				{
					fromRight.condition.wait(lock);
				}
				storage.rightG = std::move(fromRight.channel.front());
				fromRight.channel.pop();
			}
			{
				std::unique_lock<std::mutex> lock(fromLeft.mtx);
				while (fromLeft.channel.empty())
				{
					fromLeft.condition.wait(lock);
				}
				storage.leftG = std::move(fromLeft.channel.front());
				fromLeft.channel.pop();
			}
			/*computing generationsBlock (or less if generationsTotal is not divisible by generationsBlock) generations. 
			In each generation it computes only valid part of the vector. */
			for (size_t i = 1; i <= generationsBlock && i + generationIndex <= generationsTotal; i++)
			{
				for (size_t j = i; j < storage.size() - i; j++)
				{
					secondaryStorage[j] = sf(storage[j - 1], storage[j], storage[j + 1]);
				}
				std::swap(secondaryStorage, storage);
			}
		}
		//writes to result, which then will be moved to original vector once all threads finish
		{
			std::unique_lock<std::mutex> lock(mutexToWriteData);
			for (size_t i = 0; i < storage.thisPart.size(); i++)
			{
				*(res + i) = storage[i + storage.leftG.size()];
			}
		}
	}

	/*each thread creates two of this object. This class contains three parts - leftG will be recieved from left neighbor, thisPart is computed by this thread
	and rightG will be recieved from right neighbor. It can be indexed without knowing how the parts are divided.*/
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
};


