# Work stealing Thread Pool (WSTP)

A fast and lightweight work stealing thread pool based on the book "Concurrency In Action"[^1] by Anthony Williams.

## Build & install
To build, install and run the tests:

```zsh
git clone https://github.com/FerdiHummel/WorkStealingThreadPool.git
cd WorkStealingThreadPool
mkdir build && cd build
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release  ..
make install
ctest .
```
Now the package can be located by cmake using:

```CMake
find_package(WorkStealingThreadPool REQUIRED)
```

## Usage

```C++
#include "work_stealing_thread_pool.hpp"

...

WorkStealingThreadPool::work_stealing_thread_pool wstp{};
auto fun =  [&](int a){return a;};
auto future = wstp.submit(fun, 100);
future.wait();
auto result = future.get();
```

## References
[^1]: Anthony Williams (2012), Manning ISBN 9781933988771, C++ Concurrency in Action
