Assignment Description:

The purpose is to learn how to use condition variable to protect a limited size resource. A circular buffer with 15 positions (each position stores 1 character) will be used to communicate information between two threads: a producer and a consumer. 
The producer thread will read characters one by one from a buffer until the null character is reached. The input will contain no more than 50 characters. The producer must inform the consumer when it has finished placing the last character in the buffer. 
The consumer thread will read the characters one by one from the shared buffer and print them to the screen. A parent process will create both producer and consumer threads and will wait until both have finished execution.
-----------------------------
This program uses a circular buffer accessible by a consumer and producer thread. The producer thread puts chars in the buffer, and the consumer takes them out and prints them to
stdout. Since they run in parallel, they use a conditional variable and mutex locks to ensure that they do not create a race condition and attempt to access similar spots in similar
time, which would cause items to be skipped or printed multiple times. Input buffer can hold up to 150 chars, but strings of greater than 50 chars get cut off and only the first
50 chars are serviced.
