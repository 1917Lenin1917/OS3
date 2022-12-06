#include <iostream>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <random>
#include <conio.h>

using namespace std::chrono_literals;

class App {
private:
    std::queue<int> m_data;
    std::atomic_bool m_bFinished;
    std::mutex m_Mutex;
    std::condition_variable m_cv;
    int m_sleepSize;
    int m_workSize;

    std::mt19937 rnd;
    std::uniform_int_distribution<int> dist{ 1, 100 };
public:
    App(int sleepSize, int workSize)
        : m_bFinished(false)
        , m_sleepSize(sleepSize), m_workSize(workSize), rnd(std::random_device()())
    {}

    void Producer(std::chrono::milliseconds sleep_duration, int id)
    {
        bool sleep = false;
        std::cout << "Producer sleep time: " << sleep_duration.count() << "ms\n";
        while (!m_bFinished)
    	{
            std::unique_lock<std::mutex> lock(m_Mutex);
            if (sleep && m_data.size() <= m_workSize)
            {
                sleep = false;
            }
            if (!sleep)
            {
	            if (m_data.size() >= m_sleepSize)
	            {
                    sleep = true;
	            }
                else
                {
                    m_data.push(dist(rnd));
                    std::cout << "Pushed from id = " << id << ". Queue size: " << m_data.size() << "!!\n";
                }
            }
            lock.unlock();
            m_cv.notify_one();
            std::this_thread::sleep_for(sleep_duration);
        }
    }

    void Consumer()
    {
        auto sleep_duration = std::chrono::milliseconds(rand() % 200 + 400);
        do
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            while (m_data.empty())
            {
                std::cout << "Waiting... " << m_bFinished << "\n";
                //m_cv.wait(lock, [&]() { return m_bFinished == 2 || !m_data.empty(); }); // predicate an while loop - protection from spurious wakeups
                m_cv.wait(lock);
            }
            if (!m_data.empty()) 
            {
                std::cout << "Consumer Thread, queue element: " << m_data.front() << " size: " << m_data.size() << std::endl;
                m_data.pop();
                lock.unlock();
                std::this_thread::sleep_for(sleep_duration);
                lock.lock();
            }
        } while (m_bFinished && !m_data.empty() || !m_bFinished);
    }

    void driver()
    {
	    while (true)
	    {
			char c = _getch();
            if (c == 'q')
            {
                m_bFinished = true;
                std::cout << "\n\nStopping production. Waiting for consumers to finish.\n\n";

            	return;
            }
	    }
    }
};


int main()
{
    srand(time(NULL));

    
    App app(100, 80);

    auto sleep_duration1 = std::chrono::milliseconds(rand() % 200 + 100);
    auto sleep_duration2 = std::chrono::milliseconds(rand() % 200 + 100);

    std::thread consumer_thread(&App::Consumer, &app);
    std::this_thread::sleep_for(100ms);

    std::thread producer_thread1(&App::Producer, &app, sleep_duration2, 0);
    std::this_thread::sleep_for(100ms);
    std::thread producer_thread2(&App::Producer, &app, sleep_duration1, 1);

    std::thread driver_thread(&App::driver, &app);
    driver_thread.join();
    consumer_thread.join();
    producer_thread1.join();
    producer_thread2.join();
    
    return 0;
}
