#include <iostream>
#include <chrono>
#include <cstdlib>
#include <pthread.h>
#include <time.h>
#include <fstream>
#include "../skiplist.h"

#define STORE_RESULT "../stressTest/test_result"

#define NUM_THREADS 1
#define TEST_COUNT 1000000
using namespace std;

skipList<int, string> skList(18);

void *insertElement(void *threadid)
{
    long tid;
    tid = (long long)threadid;
    int tmp = TEST_COUNT / NUM_THREADS;
    for (int i = tid * tmp, count = 0; count < tmp; i++)
    {
        count++;
        skList.insertElem(rand() % TEST_COUNT, "a");
    }
    pthread_exit(NULL);
}
void *deleteElement(void *threadid)
{
    long long tid;
    tid = (long long)threadid;

    int tmp = TEST_COUNT / NUM_THREADS;
    for (int i = tid * tmp, count = 0; count < tmp; i++)
    {
        count++;
        skList.deleteElem(rand() % TEST_COUNT);
    }
    pthread_exit(NULL);
}

void *updeteElement(void *threadId)
{
    long long tId;
    tId = (long long)threadId;
    int tmp = TEST_COUNT / NUM_THREADS;
    for (int i = tId * tmp, count = 0; count < tmp; i++, count++)
    {
        skList.updateElem(rand() % TEST_COUNT, "b");
    }
    pthread_exit(nullptr);
}

void *getElement(void *threadId)
{
    long long tId;
    tId = (long long)threadId;
    int tmp = TEST_COUNT / NUM_THREADS;
    for (int i = tId * tmp, count = 0; count < tmp; i++, count++)
    {
        skList.searchElem(rand() % TEST_COUNT);
    }
    pthread_exit(nullptr);
}

int main()
{
    srand(time(nullptr));

    ofstream fileWriter;
    fileWriter.open(STORE_RESULT, ios::app);
    fileWriter << "TEST_COUNT : " << TEST_COUNT << " NUM_THREADS : " << NUM_THREADS << endl;
    fileWriter.close();

    // ²âÊÔinsert
    {
        pthread_t threads[NUM_THREADS];
        int rc;
        intptr_t i;
        auto start = chrono::high_resolution_clock::now();

        for (i = 0; i < NUM_THREADS; i++)
        {
            rc = pthread_create(&threads[i], nullptr, insertElement, (void *)i);
            if (rc)
            {
                cout << "Error:unable to create thread," << rc << endl;
                exit(-1);
            }
        }

        void *result;
        for (i = 0; i < NUM_THREADS; i++)
        {
            if (pthread_join(threads[i], &result) != 0)
            {
                perror("pthread_create() error");
                exit(3);
            }
        }

        auto finish = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = finish - start;
        cout << elapsed.count() << endl;

        fileWriter.open(STORE_RESULT, ios::app);
        fileWriter << "insert elapsed: " << elapsed.count() << endl;
        fileWriter.close();
    }

    // ²âÊÔupdate
    {
        pthread_t threads[NUM_THREADS];
        int rc;
        intptr_t i;
        auto start = chrono::high_resolution_clock::now();

        for (i = 0; i < NUM_THREADS; i++)
        {
            rc = pthread_create(&threads[i], nullptr, updeteElement, (void *)i);
            if (rc)
            {
                cout << "Error:unable to create thread," << rc << endl;
                exit(-1);
            }
        }

        void *result;
        for (i = 0; i < NUM_THREADS; i++)
        {
            if (pthread_join(threads[i], &result) != 0)
            {
                perror("pthread_create() error");
                exit(3);
            }
        }

        auto finish = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = finish - start;
        cout << elapsed.count() << endl;

        fileWriter.open(STORE_RESULT, ios::app);
        fileWriter << "updete elapsed: " << elapsed.count() << endl;
        fileWriter.close();
    }

    // ²âÊÔ²éÕÒ
    {
        pthread_t threads[NUM_THREADS];
        int rc;
        intptr_t i;
        auto start = chrono::high_resolution_clock::now();

        for (i = 0; i < NUM_THREADS; i++)
        {
            rc = pthread_create(&threads[i], nullptr, getElement, (void *)i);
            if (rc)
            {
                cout << "Error:unable to create thread," << rc << endl;
                exit(-1);
            }
        }

        void *result;
        for (i = 0; i < NUM_THREADS; i++)
        {
            if (pthread_join(threads[i], &result) != 0)
            {
                perror("pthread_create() error");
                exit(3);
            }
        }

        auto finish = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = finish - start;
        cout << elapsed.count() << endl;

        fileWriter.open(STORE_RESULT, ios::app);
        fileWriter << "search elapsed: " << elapsed.count() << endl;
        fileWriter.close();
    }

    // ²âÊÔÉ¾³ý
    {
        pthread_t threads[NUM_THREADS];
        int rc;
        intptr_t i;
        auto start = chrono::high_resolution_clock::now();

        for (i = 0; i < NUM_THREADS; i++)
        {
            rc = pthread_create(&threads[i], nullptr, deleteElement, (void *)i);
            if (rc)
            {
                cout << "Error:unable to create thread," << rc << endl;
                exit(-1);
            }
        }

        void *result;
        for (i = 0; i < NUM_THREADS; i++)
        {
            if (pthread_join(threads[i], &result) != 0)
            {
                perror("pthread_create() error");
                exit(3);
            }
        }

        auto finish = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = finish - start;
        cout << elapsed.count() << endl;

        fileWriter.open(STORE_RESULT, ios::app);
        fileWriter << "delete elapsed: " << elapsed.count() << endl;
        fileWriter.close();
    }
}