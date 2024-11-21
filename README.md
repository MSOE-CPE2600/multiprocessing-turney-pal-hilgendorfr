<!--
Name: Ryan Pal Hilgendorf
Assignment: Lab 11 - Multiprocessing
Secton: CPE 2600 121
-->

# System Programming Lab 11 Multiprocessing

The changes to mandel.c started by generating all 50 images, zooming out, to a for loop. This logic was then taken into account when forking, which produces a specific number of children all with their own assigned set of images to generate. The scale each image is at is also predetermined, so even when generating asynchronously the final results are in order.

Generated graph of results based on number of children:

![image](~/sysprog_lab11_graph.png)

You can see that the time for completion starts to increase after hitting a low at 10 children. This is likely due to the fact that the laptop the experiment was run on has 12 cores, meaning that going beyond 12 children will begin to hinder the CPU. This also means that 12 children is the sweet spot.