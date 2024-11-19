/****************************************************/
/*  CPP Template for School                         */
/*  Author: CompileErr0r(YiJia)                     */
/*  Author ID: 11127137                             */
/*  Compile Environment: Ubuntu Linux 22.04 x86_64  */
/*  Compiler: g++ 9.4                               */
/****************************************************/

#ifdef __linux__
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif
#include <chrono>
#include <climits>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#define endl '\n'

#define CompileErr0r_is_the_weakest_hacker \
    ios_base::sync_with_stdio(0);          \
    cin.tie(0);

using namespace std;

vector<vector<int> *> results;

class ParallelSorter {
   private:
    vector<int> arr;
    vector<vector<int>> splited_arr;
    mutex mtx;
    int size; // N
    int K;

   public:
    ParallelSorter(vector<int> arr, int K) {
        this->arr = arr;
        this->size = arr.size();
        this->K = K;
    }
    ~ParallelSorter() {}

    int GetSize() { return size; }

    vector<int> GetArr() { return arr; }

    /*
        Pure Bubble Sort O(N^2)
    */

    void SingleProcessSingleThreadBubbleSort() {
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size - i - 1; j++) {
                if (arr[j] > arr[j + 1]) {
                    swap(arr[j], arr[j + 1]);
                }
            }
        }
    }

    /*
        Pure Bubble Sort in [l, r)
    */
    void RangedBubbleSort(int l, int r) {
        for (int i = l; i < r; i++) {
            for (int j = l; j < r - 1; j++) {
                if (arr[j] > arr[j + 1]) {
                    swap(arr[j], arr[j + 1]);
                }
            }
        }
    }

    void PrintArr() {
        for (int i = 0; i < size; i++) {
            cout << arr[i] << " ";
        }
        cout << endl;
    }

    void MergeSplitedArr() {
        vector<int> result;
        vector<int> index(K, 0);
        for (int i = 0; i < size; i++) {
            int min_index = -1;
            int min_value = INT_MAX;
            for (int j = 0; j < K; j++) { // Choose the minimum value
                if (index[j] < splited_arr[j].size() && splited_arr[j][index[j]] < min_value) {
                    min_value = splited_arr[j][index[j]];
                    min_index = j;
                }
            }
            result.push_back(min_value);
            index[min_index]++;
        }
        arr = result;
    }

    /*
      Split the array into K parts, and sort each part in pure bubble sort(no
      multi thread) then merge them together in the same process
     */
    void SingleProcessBubbleMergeSort() {
        int part_size = size / K;
        if (size % K != 0) part_size++;
        for (int i = 0; i < K; i++) {
            int l = i * part_size;
            int r = (i == K - 1) ? size : (i + 1) * part_size;
            // cerr << "l: " << l << " r: " << r << endl;
            RangedBubbleSort(l, r);
            vector<int> temp;
            for (int j = l; j < r; j++) {
                temp.push_back(arr[j]);
            }
            splited_arr.push_back(temp);
        }

        MergeSplitedArr();
    }

    vector<int> Merge(vector<int> left, vector<int> right) {
        vector<int> result;
        int i = 0, j = 0;
        while (i < left.size() && j < right.size()) {
            if (left[i] < right[j]) {
                result.push_back(left[i]);
                i++;
            } else {
                result.push_back(right[j]);
                j++;
            }
        }
        while (i < left.size()) {
            result.push_back(left[i]);
            i++;
        }
        while (j < right.size()) {
            result.push_back(right[j]);
            j++;
        }
        return result;
    }

    void MergeRef(vector<int> *result, vector<int> left, vector<int> right) {
        // mtx.lock();
        int i = 0, j = 0;
        while (i < left.size() && j < right.size()) {
            if (!left.empty() && left[i] < right[j]) {
                result->push_back(left[i]);
                i++;
            } else {
                result->push_back(right[j]);
                j++;
            }
        }
        while (!left.empty() && i < left.size()) {
            result->push_back(left[i]);
            i++;
        }
        while (!right.empty() && j < right.size()) {
            result->push_back(right[j]);
            j++;
        }
        // mtx.unlock();
    }

    void SplitedBubbleSort(vector<int> &splited) {
        for (int i = 0; i < splited.size(); i++) {
            for (int j = 0; j < splited.size() - i - 1; j++) {
                if (splited[j] > splited[j + 1]) {
                    swap(splited[j], splited[j + 1]);
                }
            }
        }
    }

    void SplitArray() {
        int part_size = size / K;
        if (size % K != 0) part_size++;
        for (int i = 0; i < K; i++) {
            int l = i * part_size;
            int r = (i == K - 1) ? size : (i + 1) * part_size;
            vector<int> temp;
            for (int j = l; j < r; j++) {
                temp.push_back(arr[j]);
            }
            splited_arr.push_back(temp);
        }
    }

    void WaitProcess(vector<pid_t> pids) {
        for (int i = 0; i < pids.size(); i++) {
            if (pids[i] < 0) {
                cout << "Error: Fork failed" << endl;
                exit(1);
            }
            waitpid(pids[i], NULL, 0);
        }
    }

    void MultiProcessBubbleSort() {
        vector<pid_t> pids;
        for (int i = 0; i < K; i++) {
            int fd[2];
            if (pipe(fd) < 0) {
                cout << "Error: Pipe failed" << endl;
                exit(1);
            }
            pid_t pid = fork();
            if (pid == 0) {
                close(fd[0]);
                SplitedBubbleSort(splited_arr[i]);
                for (int j = 0; j < splited_arr[i].size(); j++) {
                    write(fd[1], &splited_arr[i][j], sizeof(int));
                }
                close(fd[1]);
                exit(0);
            } else if (pid < 0) {
                cout << "Error: Fork failed" << endl;
                exit(1);
            }

            pids.push_back(pid);
            close(fd[1]);
            vector<int> temp;
            int temp_int;
            while (read(fd[0], &temp_int, sizeof(int)) > 0) {
                temp.push_back(temp_int);
            }
            splited_arr[i] = temp;
            close(fd[0]);
        }
        WaitProcess(pids);
    }

    vector<int> MultiProcessMergeSort() {
        while (splited_arr.size() > 1) {
            vector<pid_t> pids;
            vector<vector<int>> newArr;
            vector<vector<int>> tempNewArr;
            for (int i = 0; i < (int)splited_arr.size(); i += 2) {
                if (i + 1 < splited_arr.size()) {
                    int fd[2];
                    if (pipe(fd) < 0) {
                        cout << "Error: Pipe failed" << endl;
                        exit(1);
                    }
                    pid_t pid = fork();
                    if (pid == 0) {
                        close(fd[0]);
                        vector<int> result;
                        result = Merge(splited_arr[i], splited_arr[i + 1]);
                        for (int j = 0; j < result.size(); j++) {
                            write(fd[1], &result[j], sizeof(int));
                        }
                        close(fd[1]);
                        exit(0);
                    } else if (pid < 0) {
                        cout << "Error: Fork failed" << endl;
                        exit(1);
                    }

                    pids.push_back(pid);
                    close(fd[1]);
                    vector<int> temp;
                    int temp_int;
                    while (read(fd[0], &temp_int, sizeof(int)) > 0) {
                        temp.push_back(temp_int);
                    }
                    newArr.push_back(temp);
                    close(fd[0]);
                } else {
                    tempNewArr.push_back(splited_arr[i]);
                }
            }

            WaitProcess(pids);

            for (int i = 0; i < tempNewArr.size(); i++) {
                newArr.push_back(tempNewArr[i]);
            }

            splited_arr = newArr;
        }
        return splited_arr[0];
    }

    void MultiProcessBubbleMergeSort() {
        SplitArray();
        MultiProcessBubbleSort();
        if (K > 1) {
            arr = MultiProcessMergeSort();
        }
    }

    void MultiThreadBubbleSort() {
        vector<thread> threads;
        for (int i = 0; i < splited_arr.size(); i++) {
            threads.push_back(thread(&ParallelSorter::SplitedBubbleSort, this, ref(splited_arr[i])));
        }

        for (int i = 0; i < K; i++) {
            threads[i].join();
        }
    }

    vector<int> MultiThreadMergeSort() {
        while (splited_arr.size() > 1) {
            vector<thread> threads;
            vector<vector<int>> newArr;
            vector<vector<int>> tempNewArr;
            // vector<vector<int> *> results;
            results.clear();
            for (int i = 0; i < (int)splited_arr.size(); i += 2) {
                if (i + 1 < splited_arr.size()) {
                    results.push_back(new vector<int>());
                    threads.push_back(thread(&ParallelSorter::MergeRef, this, results.back(), splited_arr[i],
                                             splited_arr[i + 1]));

                } else {
                    tempNewArr.push_back(splited_arr[i]);
                }
            }

            for (int i = 0; i < threads.size(); i++) {
                threads[i].join();
                newArr.push_back(*results[i]);
            }

            for (int i = 0; i < tempNewArr.size(); i++) {
                newArr.push_back(tempNewArr[i]);
            }

            for (int i = 0; i < results.size(); i++) {
                delete results[i];
                results[i] = nullptr;
            }

            splited_arr = newArr;
        }
        return splited_arr[0];
    }

    void MultiThreadBubbleMergeSort() {
        SplitArray();
        MultiThreadBubbleSort();
        if (K > 1)
            arr = MultiThreadMergeSort();
        else
            arr = splited_arr[0];
    }
};

class Tasks {
   private:
    string input_file_name;
    ParallelSorter *sorter;

   public:
    void ReadFile(string input_file_name, int K) {
        this->input_file_name = input_file_name;
        ifstream file(input_file_name + ".txt");
        if (!file.is_open()) {
            cout << "Error: File not found" << endl;
            exit(1);
        }

        vector<int> arr;
        int temp;
        while (file >> temp) {
            arr.push_back(temp);
        }

        sorter = new ParallelSorter(arr, K);
    }

    double Sort(int case_num) {
        clock_t start, end;
        if (case_num == 1) {
            start = clock();
            sorter->SingleProcessSingleThreadBubbleSort();
            end = clock();
        } else if (case_num == 2) {
            start = clock();
            sorter->SingleProcessBubbleMergeSort();
            end = clock();
        } else if (case_num == 3) {
            start = clock();
            sorter->MultiProcessBubbleMergeSort();
            end = clock();
        } else if (case_num == 4) {
            start = clock();
            sorter->MultiThreadBubbleMergeSort();
            end = clock();
        } else {
            cout << "Error: Invalid case number" << endl;
            exit(1);
        }
        return (double)(end - start) / CLOCKS_PER_SEC;
    }

    string GetCurrentTime() {
        time_t now = time(0);
        tm *ltm = localtime(&now);

        char buffer[80];
        // yyyy-mm-dd hh:mm:ss+08:00
        strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", ltm);
        string time(buffer);
        time += "+08:00";

        return time;
    }

    void WriteFile(int case_num, double cpu_time) {
        string output_file_name = input_file_name + "_output" + to_string(case_num) + ".txt";
        int size = sorter->GetSize();
        vector<int> arr = sorter->GetArr();
        ofstream file(output_file_name);
        if (!file.is_open()) {
            cout << "Error: Cannot create output file" << endl;
            exit(1);
        }
        cout << "Sort : \n";
        file << "Sort : \n";
        for (int i = 0; i < size; i++) {
            cout << arr[i] << "\n";
            file << arr[i] << "\n";
        }
        cout << "CPU Time : " << fixed << cpu_time << endl;
        file << "CPU Time : " << fixed << cpu_time << endl;
        string time = GetCurrentTime();
        cout << "Output Time : " << time << endl;
        file << "Output Time : " << time;
    }

    ~Tasks() {
        delete sorter;
        sorter = nullptr;
    }
} task;

int main() {
    string input;
    cout << "請輸入檔案名稱: \n";
    cin >> input;
    cout << "請輸入要切成幾份: \n";
    int K;
    cin >> K;
    task.ReadFile(input, K);
    int case_num = 1;
    cout << "請輸入方法編號:(方法1, 方法2, 方法3, 方法4) \n";
    cin >> case_num;
    double cpu_time = task.Sort(case_num);
    task.WriteFile(case_num, cpu_time);

    cout << flush;
    return 0;
}