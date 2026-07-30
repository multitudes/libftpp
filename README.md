# 42-libftpp

## What is a Pool?

In software engineering, an Object Pool is a creational design pattern used to manage performance and memory.
Dynamically allocating (using new) and deallocating (using delete) memory during runtime is computationally expensive. If you are building a system that frequently creates and destroys small objects—like bullets in a video game, or network packets—that overhead will slow your program down.
A Pool solves this by pre-allocating a large batch (a "pool") of memory for these objects upfront. According to the subject, your Pool class "manages a collection of reusable templated TType objects". Instead of creating a new object from scratch, you "acquire" an existing, unused chunk of memory from the pool, construct your object there, and when you are done, you give the memory back to the pool to be reused.  
Crucially, when the object is released back to the pool, the subject requires "calling the destructor of the TType object but without deallocating the memory". The memory stays reserved for the next time you need it.  

- Variadic Templates and Perfect Forwarding: The subject asks to implement the acquire method 

```cpp
void resize(const size_t& numberOfObjectStored):

// Allocates a certain number of TType objects withing the Pool.
template<typename ... TArgs> Pool::Object<TType>
acquire(TArgs&& p_args): 

// Creates a Pool::Object containing a pre-allocated object, 
// using the constructor with parameters as defined by TArgs definition.

// Pool::Object :
-TType* operator -> (): 
// Returns the pointer stored within the Pool::Object.
```
 `template<typename TArgs> TArgs&& p_args`. The hints explicitly mention looking into variadic templates to handle passing an arbitrary number of arguments directly to the object's constructor.