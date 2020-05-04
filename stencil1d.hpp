#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <queue>

template<typename ET>
class circle
{
public:
	circle(std::size_t s) :field(std::vector<ET>(s)), secondaryField(std::vector<ET>(s)) {}

	std::size_t size() const {
		return field.size();
	}
	void set(std::ptrdiff_t x, const ET& v) {
		std::ptrdiff_t index = (x + size()) % size();
		field[index] = v;
	}
	decltype(auto) get(std::ptrdiff_t x) const {
		std::ptrdiff_t index = (x + size()) % size();
		return field[index];
	}
	template<typename SF>
	void run(SF&& sf, std::size_t g, std::size_t thrs = std::thread::hardware_concurrency()) {
		std::vector<std::thread> workers(thrs);
		size_t W = size() / thrs;
		size_t G = W / 16;
		auto startingPosition = field.begin();
		for (size_t th = 0; th < thrs; th++) {
			if (size() % thrs > th) {
				workers[th] = std::thread(&circle::oneThreadJob, startingPosition, startingPosition + W + 1, G, sf, g, th);
				startingPosition += (W + 1);
			}
			else
			{
				workers[th] = std::thread(&circle::oneThreadJob, startingPosition, startingPosition + W, G, sf, g, th);
				startingPosition += W;
			}
		}
	}
	class threeVectors{
	public:
		ET operator[](size_t index) {
			if (index < leftG.size()) {
				return leftG[index];
			}
			else if (index < leftG.size() + thisPart.size())
			{
				return thisPart[index - leftG.size()];
			}
			else {
				return rightG[index - leftG.size() - thisPart.size()];
			}

		}

		std::vector<ET> leftG;
		std::vector<ET> thisPart;
		std::vector<ET> rightG;
	};
	template<typename SF>
	void oneThreadJob(typename std::vector<ET>::iterator beg, typename std::vector<ET>::iterator end, size_t G, SF&& sf, size_t g, size_t which)
	{

		size_t q; // @need - number of threads
		threeVectors storage;
		storage.thisPart(beg, end);
		while (g > 0) {
			//sending
			{
				std::lock_guard<std::mutex> lock(mutexes[2 * which]);
				channels[2 * which].push_back(storage.thisPart.begin(), storage.thisPart.begin() + G);
				notifications[2 * which] = true;
			}
			cond_variables[2 * which].notify_one();

			{
				std::lock_guard<std::mutex> lock(mutexes[2 * which + 1]);
				channels[2 * which + 1].push_back(storage.thisPart.end() - G, storage.thisPart.end()); // @todo: around the circle
				notifications[2 * which + 1] = true;
			}
			cond_variables[2 * which + 1].notify_one();

			//recieving
			size_t indexRight = ((which + 1) % q) * 2;
			std::unique_lock<std::mutex> lock(mutexes[indexRight]);
			while (!notifications[indexRight]) {
				cond_variables[indexRight].wait(lock);
			}
			if (!channels[indexRight].empty()) {
				storage.rightG = channels[indexRight].front();
				channels[indexRight].pop();
			}
			notifications[indexRight] = false;


			size_t indexLeft = ((which - 1 + q) % q) * 2 + 1;
			std::unique_lock<std::mutex> lock(mutexes[indexLeft]);
			while (!notifications[indexLeft]) {
				cond_variables[indexLeft].wait(lock);
			}
			if (!channels[indexLeft].empty()) {
				storage.leftG = channels[indexLeft].front();
				channels[indexLeft].pop();
			}


			notifications[indexLeft] = false;
			for (size_t i = 0; i < G && i < g; i++)
			{
				
			}
			g -= G;
		}


	}
private:
	std::vector<ET> field;
	std::vector<std::mutex> mutexes;
	std::vector<std::condition_variable> cond_variables;
	std::vector<bool> done;
	std::vector<bool> notifications;
	std::vector<std::queue<std::vector<ET>>> channels;
	std::vector<ET> secondaryField;

};
