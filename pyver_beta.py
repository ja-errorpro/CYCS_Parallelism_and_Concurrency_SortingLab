import threading
import multiprocessing
import time


original_list = []
splitted_list = []
sorted_list = []

def read_file():
    global original_list
    with open(input('請輸入檔案名稱：\n') + '.txt', 'r') as file:
        original_list = file.read().splitlines()
        for i in range(0, len(original_list)):
            original_list[i] = int(original_list[i])

def bubble_sort(lst):
    n = len(lst)
    for i in range(n):
        for j in range(0, n-i-1):
            if lst[j] > lst[j+1]:
                lst[j], lst[j+1] = lst[j+1], lst[j]
    return lst

def merge_sort(lst1, lst2):
    global sorted_list
    i = j = 0
    while i < len(lst1) and j < len(lst2):
        if lst1[i] < lst2[j]:
            sorted_list.append(lst1[i])
            i += 1
        else:
            sorted_list.append(lst2[j])
            j += 1
    sorted_list += lst1[i:]
    sorted_list += lst2[j:]

def split_list(lst, k):
    global splitted_list
    n = len(lst)
    for i in range(0, n, k):
        splitted_list.append(lst[i:i+k])

def Task1_PureBubbleSort():
    global original_list
    bubble_sort(original_list)

def Task2_SingleThreadBubbleMergeSort(k: int):
    global splitted_list
    global sorted_list
    split_list(original_list, k)
    for i in range(len(splitted_list)):
        bubble_sort(splitted_list[i])
    while len(splitted_list) > 1:
        merge_sort(splitted_list[0], splitted_list[1])
        splitted_list = splitted_list[2:]
        splitted_list.append(sorted_list)
        sorted_list = []
    splitted_list = splitted_list[0]

def Task3_MultiProcessBubbleMergeSort(k: int): # k processes to bubble sort, k-1 processes to merge sort
    global splitted_list
    global sorted_list
    split_list(original_list, k)
    process_list = []
    for i in range(len(splitted_list)):
        splitted_list[i] = multiprocessing.Array('i', splitted_list[i])
        process_list.append(multiprocessing.Process(target=bubble_sort, args=(splitted_list[i],)))
        process_list[i].start()
    for i in range(len(process_list)):
        process_list[i].join()

    while len(splitted_list) > 1:
        process_list = []
        for i in range(0, len(splitted_list), 2):
            if i+1 < len(splitted_list):
                splitted_list[i] = splitted_list[i].tolist()
                splitted_list[i+1] = splitted_list[i+1].tolist()
                splitted_list[i] = multiprocessing.Array('i', splitted_list[i])
                splitted_list[i+1] = multiprocessing.Array('i', splitted_list[i+1])
                process_list.append(multiprocessing.Process(target=merge_sort, args=(splitted_list[i], splitted_list[i+1])))
                process_list[-1].start()


        for i in range(len(process_list)):
            process_list[i].join()
        splitted_list = splitted_list[::2]
        splitted_list.append(sorted_list)
        sorted_list = []
    splitted_list = splitted_list[0]

def Task4_MultiThreadBubbleMergeSort(k: int): # k threads to bubble sort, k-1 threads to merge sort
    global splitted_list
    global sorted_list
    split_list(original_list, k)
    thread_list = []
    for i in range(len(splitted_list)):
        thread_list.append(threading.Thread(target=bubble_sort, args=(splitted_list[i],)))
        thread_list[i].start()
    for i in range(len(thread_list)):
        thread_list[i].join()

    while len(splitted_list) > 1:
        thread_list = []
        for i in range(0, len(splitted_list), 2):
            if i+1 < len(splitted_list):
                thread_list.append(threading.Thread(target=merge_sort, args=(splitted_list[i], splitted_list[i+1])))
                thread_list[-1].start()
                
        for i in range(len(thread_list)):
            thread_list[i].join()
        splitted_list = splitted_list[::2]
        splitted_list.append(sorted_list)
        sorted_list = []
    splitted_list = splitted_list[0]

def main():
    read_file()
    print('請問要切成幾份：')
    k = int(input())
    print('請問要使用哪種方法：')
    method = int(input())
    if method == 1:
        start = time.time()
        Task1_PureBubbleSort()
        end = time.time()
        print('Task1_PureBubbleSort: ', end - start)
    elif method == 2:
        start = time.time()
        Task2_SingleThreadBubbleMergeSort(k)
        end = time.time()
        print('Task2_SingleThreadBubbleMergeSort: ', end - start)
    elif method == 3:
        start = time.time()
        Task3_MultiProcessBubbleMergeSort(k)
        end = time.time()
        print('Task3_MultiProcessBubbleMergeSort: ', end - start)
    elif method == 4:
        start = time.time()
        Task4_MultiThreadBubbleMergeSort(k)
        end = time.time()
        print('Task4_MultiThreadBubbleMergeSort: ', end - start)
    with open('output.txt', 'w') as file:
        for i in range(0, len(splitted_list)):
            file.write(str(splitted_list[i]) + '\n')

if __name__ == '__main__':
    main()


    