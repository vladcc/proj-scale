# proj-scale
scalable lock free concurrency benchmark

The benchmark creates a data set of N integers. The main thread creates actor
threads, each with a lock free queue. The main thread communicates with each
actor through these lock free queues by feeding them the data set one integer at
a time. Each actor copies each input integer into a vector, one vector per
actor. At the end of the test all vectors are compared to the data set and if
the same the elapsed wall clock time is reported. If they are not the same,
there's a bug.

The number of actors, the size of their queues, and the size of the data set
are given on the command line. If an additional argument is given, the test
blocks before execution, so the user has a chance to pin all actors.
