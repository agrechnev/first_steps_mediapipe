First steps with Google MediaPipe: An Informal tutorial
========

By Oleksiy Grechnyev, IT-JIM (https://www.it-jim.com/)  

Development in progress, more is coming soon hopefully ...  

What is this?
-----------

This unofficial tutorial gives a gentle introduction into the Google MediaPipe C++ API.

Google MediaPipe (MP) is fun, but in my opinion the official docs are insufficient,
especially if you are interested in the core C++ API. But the C++ API is what MP is
really about, the "Solutions" and language bindings are just thin wrappers. 

What you will learn:  
* How to write MP pipelines in C++, send and receive data packets  
* How to write your own MP calculators  
* How to use standard MP calculators (only a few examples)  

What you will NOT learn from this tutorial:  
* MediaPipe solutions  
* Languages and platforms other than C++/desktop  
* Audio processing  
* GPU usage  
* Neural network inference in TF/TF Lite (but maybe I'll come back to that later)  
* GLog, Google Test, and other Google goodies not crucial for understanding MediaPipe  


Prerequisites:  
* Fluent C++ knowledge  
* Minimal knowledge of Bazel, OpenCV, Protobuf  
* Read official MP docs (Sections "MediaPipe in C++" and "Framework Concepts")  
* Install MP and try the MP C++ "Hello world" example

Enjoy!

Installation
------------

This has been tested on Kubuntu 20.10 and later 21.04, but hopefully it works on other OSes as well.

1. Install Bazel and MediaPipe as per official MP docs.  
2. Check that hello world works. In the MP repo root directory, type:  
`export GLOG_logtostderr=1`  
`bazel run --define MEDIAPIPE_DISABLE_GPU=1 //mediapipe/examples/desktop/hello_world`  
Important: On modern Linux you will see weird errors if you don't have `python` in your path, `python3` will not do!  
This is fixed by `sudo apt install python-is-python2`.  
If the standard hello world does not work for you, I cannot help you further.  
3. Copy the directory `first_steps` from this repo to `mediapipe/examples` in the MP repo (in will sit next to `android`, `coral`, `desktop` and `ios`).  
4. Upon upgrading to opencv 4.5 (with Kubuntu 21.04), I had to comment out the line `#include <opencv2/cvconfig.h>` in
`mediapipe/framework/port/opencv_core.inc`.

Note: I worked on MediaPipe commit ea8d45731f5a052f79745e35bfd8240d6ac568d2 aka tag 0.8.2, did not test on other
MP versions, minor changes might be needed (but hopefully not).

To run an example `1_1` (for instance) type:  
`bazel run --define MEDIAPIPE_DISABLE_GPU=1 //mediapipe/examples/first_steps/1_1`  

To run an example with additional files needed at runtime (curently it's only 1_3) type:  
`bazel build --define MEDIAPIPE_DISABLE_GPU=1 //mediapipe/examples/first_steps/1_3`  
`bazel-bin/mediapipe/examples/first_steps/1_3/1_3`  

Examples
---------

Each example introduces some new mediapipe concept. They have detailed comments. Please follow them in order, as
example `1_2` assumes you are already familiar with `1_1`. Currently the following examples are available:

1.1: Simplest mediapipe pipeline  
1.2: Our first custom calculator  
1.3: Joining strings, synchronization, source  
1.4: Calculator options  
1.5: Calculator with side packets  

2.1: Video pipeline  
2.2: Video pipeline with ImageCroppingCalculator and ScaleImageCalculator  
2.3: Video pipeline with ImageCroppingCalculator (dynamic crop)  
2.4: Video pipeline with FeatureDetectorCalculator and custom image processing  

3.1: Why MediaPipe is not real-time?
3.2: Packet loss with FlowLimiterCalculator

Why Bazel?
--------

You might have noticed that all this is very different from what usual C++ programming look like.  
Where is "sudo apt install libmediapipe-dev" (sarcasm) ? Where is CMake?  

They say Google technologies do not work outside of Google, and having written this tutorial I totally agree.  

There is currently no way to use MP without Bazel (at least for beginners). In particular, you canot
create an "MP release" as a .SO library + headers. Using MP as an external
Bazel dependency is also *very* tricky (although you can find examples on the net).

This, of course, makes using MediaPipe *very*, *very* inconvenient. For all practical purposes (again, at least for beginners),
the only way to use MP in your own software project, is to build your project *inside the MediaPipe tree*, just
as we did with our `first_steps`. Naturally, this means that your entire project must be a Bazel project,
and you will have to look for any other (system-wide) external libraries (e.g. Boost or Qt) on your own (hardcoded paths or
custom shell scripts), as the usual tools like CMake find_package or pkgconfig are not available in Bazel.

Finally, is there any IDE for Bazel C++ projects? CLion plugin is advertized, but it does not work with modern CLion versions.
VS code also has Bazel plugins, which did not work for me either. However, if I open the entire MP tree as a single
VS Code project, VS Code can (mostly) find MP headers, and thus the "Show symbol definition" function works.
To me this is the main reason to use IDE (as opposed to a text editor). So I edited the code in VS Code, and executed it from the terminal.
Downside: VS code runs `cpptools` almost non-stop, taking computer resources.

I wonder what IDE do they use in Google?
