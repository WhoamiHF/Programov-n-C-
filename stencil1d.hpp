#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
template<typename ET>
class circle;

template<typename ET, typename SF>
void forwarder(void* context, typename std::vector<ET>::iterator beg, typename std::vector<ET>::iterator end, size_t G, SF&& sf, size_t g, size_t index, size_t threads)
{
	static_cast<circle<ET>*>(context)->oneThreadJob(beg, end, G, sf, g, index, threads);
}

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
	//run divides the field to threads
	template<typename SF>
	void run(SF&& sf, std::size_t g, std::size_t thrs = std::thread::hardware_concurrency()) 
	{
		std::vector<std::thread> workers(thrs);
		size_t W = size() / thrs;
		size_t G = W / 16;
		mutexesToLeft = std::vector<std::mutex>(thrs);
		mutexesToRight = std::vector<std::mutex>(thrs);
		cond_variables_to_left = std::vector<std::condition_variable>(thrs);
		cond_variables_to_right = std::vector<std::condition_variable>(thrs);
		notificationsToLeft = std::vector<bool>(thrs);
		notificationsToRight = std::vector<bool>(thrs);
		channelsToLeft = std::vector<std::queue<std::vector<ET>>>(thrs);
		channelsToRight = std::vector<std::queue<std::vector<ET>>>(thrs);

		auto startingPosition = field.begin();
		//&circle<ET>:: template oneThreadJob<SF>
		//this->oneThreadJob<SF>
		for (size_t th = 0; th < thrs; th++) 
		{
			if (size() % thrs > th) 
			{
				workers[th] = std::thread(&forwarder<ET,SF>,startingPosition, startingPosition + W + 1, G, std::ref(sf), g, th, thrs);
				//workers[th] = std::thread(&circle<ET>:: template oneThreadJob<SF>, startingPosition, startingPosition + W + 1, G, std::ref(sf), g, th, thrs);
				startingPosition += (W + 1);
			}
			else
			{
				//workers[th] = std::thread(&circle<ET>:: template oneThreadJob<SF>, startingPosition, startingPosition + W, G, std::ref(sf), g, th, thrs);
				startingPosition += W;
			}
		}
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
	//this is the function which does each thread. First it sends messages to its neighbors, then recieves from them and then does G (or less) generations. 
	template<typename SF>
	void oneThreadJob(typename std::vector<ET>::iterator beg, typename std::vector<ET>::iterator end, size_t G, SF&& sf, size_t g, size_t index, size_t threads)
	{

		threeVectors storage;
		storage.thisPart = std::vector(beg, end);

		//Creates second threeVectors, set them on correct sizes, allocates once
		threeVectors secondaryStorage;
		secondaryStorage.leftG = std::vector<ET>(G);
		secondaryStorage.thisPart = std::vector<ET>(storage.thisPart.size());
		secondaryStorage.rightG = std::vector<ET>(G);

		while (g > 0) 
		{

			//sending
			{
				std::lock_guard<std::mutex> lock(mutexesToLeft[index]);
				channelsToLeft[index].push(std::vector<ET>(storage.thisPart.begin(), storage.thisPart.begin() + G));
				notificationsToLeft[index] = true;
			}
			cond_variables_to_left[index].notify_one();
			// uses index of thread, for each two neighbor threads there are two channels - toLeft and toRight
			{
				std::lock_guard<std::mutex> lock(mutexesToRight[index]);
				channelsToRight[index].push(std::vector<ET>(storage.thisPart.end() - G, storage.thisPart.end()));
				notificationsToRight[index] = true;
			}
			cond_variables_to_right[index].notify_one();

			//recieving   @todo:notifications - not needed

			{
				size_t indexRight = (index + 1) % threads;
				std::unique_lock<std::mutex> lock(mutexesToLeft[indexRight]);
				while (!notificationsToLeft[indexRight]) 
				{
					cond_variables_to_left[indexRight].wait(lock);
				}
				if (!channelsToLeft[indexRight].empty()) 
				{
					storage.rightG = channelsToLeft[indexRight].front();
					channelsToLeft[indexRight].pop();
				}
				notificationsToLeft[indexRight] = false;
			}

			{
				size_t indexLeft = (index - 1 + threads) % threads;
				std::unique_lock<std::mutex> lock2(mutexesToRight[indexLeft]);
				while (!notificationsToRight[indexLeft]) 
				{
					cond_variables_to_right[indexLeft].wait(lock2);
				}
				if (!channelsToRight[indexLeft].empty()) 
				{
					storage.leftG = channelsToRight[indexLeft].front();
					channelsToRight[indexLeft].pop();
				}
				notificationsToRight[indexLeft] = false;
			}

			//computing G (or less) generations. In each generation it computes only valid part of the vector. 
			for (size_t i = 1; i <= G && i <= g; i++)
			{
				for (size_t j = i; j < storage.size() - i; j++)
				{
					secondaryStorage[j] = sf(storage[j - 1], storage[j], storage[j + 1]);
				}
				std::swap(secondaryStorage, storage);
			}
			g -= G;
		}
		//@todo write back -> warning write with caution
		for (size_t i = 0; i < storage.thisPart.size(); i++) 
		{
			*(beg + i) = storage[i + storage.leftG.size()];
		}
		//@todo clean (/ local in run)

	}
private:
	std::vector<ET> field;
	std::vector<std::mutex> mutexesToRight;
	std::vector<std::mutex> mutexesToLeft;
	std::vector<std::condition_variable> cond_variables_to_right;
	std::vector<std::condition_variable> cond_variables_to_left;
	std::vector<bool> notificationsToRight;
	std::vector<bool> notificationsToLeft;
	std::vector<std::queue<std::vector<ET>>> channelsToRight;
	std::vector<std::queue<std::vector<ET>>> channelsToLeft;

};


