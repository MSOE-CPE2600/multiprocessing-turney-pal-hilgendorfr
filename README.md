<!--
Name: Ryan Pal Hilgendorf
Assignment: Lab 11 & 12
Secton: CPE 2600 121
-->

# System Programming Lab 11 Multiprocessing

The changes to mandel.c started by generating all 50 images, zooming out, with a for loop. This logic was then taken into account when forking, which produces a specific number of children all with their own assigned set of images to generate. The scale each image is at is also predetermined, so even when generating asynchronously the final results are in order.

### Generated graph of results based on number of children:

![graphLab11](./sysprog_lab11_graph.png)

You can see that the time for completion starts to increase after hitting a low at 10 children. This is likely due to the fact that the laptop the experiment was run on has 12 cores, meaning that going beyond 12 children will begin to hinder the CPU. This also means that 12 children is the sweet spot, but it is not present in this experiment.

Note as of completing lab 12: I realize this is probably not correct, but it was my initial analysis so it's staying :)

---

# System Programming Lab 12 Multithreading

This time, the method compute_image() was modified. It was completely changed to decide the data, create, and finally join each thread. This data is then taken to a new method called threading() where the number of rows a thread will compute is decided, the start and end points are decided, and the original calculations from compute_image() are done instead.

### Multithreading results based on number of threads and children:

![graphLab12](./sysprog_lab12_table.png)

Multithreading seemed to impact the runtime far more than multiprocessing alone, and together they significantly improve processing speed. The table above shows my testing results. As it turns out, Linux doesn't like threading uneven numbers of threads so odd threads actually hurt performance. I personally found the best time to be from running 12 children with 16 threads. This was not necessarily the same case with my peers.