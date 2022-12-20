Hey! the repo isn't ready yet, I just made it public so I could write the wiki, please come back later!

# CPP-Flow
A blog post in a repository form about my experience programming in a ***CPS*** (Continuation-passing style)

## What's CPS exactly?
To tell you the truth, I found out that CPS had a name (and a [wiki page](https://en.wikipedia.org/wiki/Continuation-passing_style)) while writing this readme, so I don't have much dry information to give you, but, I can show an example!\
Let's say that we want to create a resource and do some operation on this resource, If I will give you the task to do so I believe that you implement this something like that:
```cpp
#include <iostream>

#include <resource_direct.hpp>

void my_task()
{
	const auto res = create_resource();
	if (!res)
	{
		std::cout << "Failed to open the resource: " << res.err() << std::end;
	}

	some_resource_logic(res);
}
```

Now the code above is great and all, but let's see how will it be implemented in CPS using the utilities in this repo
```cpp
#include <iostream>

#include <cpp-flow/composition.hpp>

#include <resource_cps.hpp>

void my_task()
{
	create_resource{} >> cppflow::composition::flow +
	[](const resource res) 
	{ 
		some_resource_logic(res); 
	} + 
	[](const resource_error error) 
	{ 
		std::cout << "Failed to open the resource: " << error << std::end; 
	};
}

```
So as you can see, the difference is that instead of having the caller of procedure X (`create_resource` in our case) handle the result of the function in CPS the caller supplies a *continuation* functor that can handle the result.

## Okay... cool but why would I want that?
There are many practical advantages to writing in CPS (and many disadvantages), but the reason that I decided to explore this way of programming is that I wasn't happy with the current tools we have for resource management.\
Basically, as a C++ programmer, I found myself writing a lot of wrappers to C functions, and with every wrapper that I have implemented/used something just felt wrong about the return value of those functions; The common way to implement a wrapper to a C function is to return some sort of a `unique_resource` object, e.g.
```cpp

// The function that we are about to wrap
int /* error_code */ create_resource_c(void** resource);

struct Resource
{
	Resource(void* res); // create from raw resource
	Resource(const std::string& err);; // create invalid resource
	~Resource();
	
	// basic unique_X declerations
	Resource(const Resource&) = delete;
	Resource(Resource&&);
	Resource& operator= (const Resource&) = delete;
	Resource& operator= (Resource&&);

	operator bool () const; // validity tester
	const std::string& err() const; // error getter

	void* raw_resource;
	std::string error;
};

Resource create_resource()
{
	void* resource;
	const auto err = create_resource_c(&resource);
	if (err != 0) return { std::format("some error: {}", err) };
	
	return { resource };
}
```

Well, I have multiple issues with this snippet:
* *Inability to pick the directly use the storage method of your choosing*: What if we don't want a `unique` style resource, maybe we want a shared resource or even a raw, of course, you can reset the unique resource, and move it to the appropriate smart resource, but then we are facing the issue of code correctness where instead of the code simply doing\
`Create resource` -> `Put in the appropriate smart resource` -> `Do whatever`\
we are implementing the following code:\
`Create unique resource` -> `Steal resource to the desired smart resource` -> `Do whatever` -> `Destruct the unnecessary unique resource`

* *error information support requires unnecessary storage*: We need storage for the error descriptor even when the creation was successful, and storage for the resource is required even when the wrapper itself failed. here we are storing the error descriptor inside the `Resource` class, but even if we will move this variable to an out-ref-parameter, the variable still needs to be stored somewhere, syntactically it could have been better with c# style out param declarator sugar-syntax, but we don't have it in C++ and again we are facing the same issue of code correctness for the failure flow:\
`Resource creation returns an error` -> `print the error`\
becomes\
`Create temporary error variable` -> `Resource creation returns an error` -> `print the temporary error variable`\
\
this also forces us to implement "unique style" resource as an "optional resource" because we are forced to create the unique resource even when we failed

* *revalidating the state*: The result of the resource creation needs to be tested at least twice, once in the wrapper, and once in the caller, basically every link in the call chain needs to test for an error after calling the next link in the chain

## So CPS solves those problems?
I believe so, lets look at the code!

```cpp
// The function that we are about to wrap
int /* error_code */ create_resource_c(void** resource);

// smart resource types definitions
struct Resource
{
	Resource(void* res);
	Resource() = delete;
	Resource(const Resource&) = delete;
	Resource& operator= (const Resource&) = delete;
	~Resource();

	void* raw_resource;
};

struct SharedResrouce
{
	SharedResrouce(void* res);
	SharedResrouce(Resource);
	SharedResrouce(const SharedResrouce&);
	SharedResrouce(SharedResrouce&&);
	~SharedResrouce();

	Resource* resource;
};

struct OptionalResource
{
	OptionalResource();
	OptionalResource(void* res);
	OptionalResource(Resource);

	OptionalResource(const OptionalResource&) = delete;
	OptionalResource(OptionalResource&&);

	~OptionalResource();

	OptionalResource& operator= (void* res);
	OptionalResource& operator= (Resource res);
	OptionalResource& operator= (OptionalResource res);

	operator bool () const;
	void* reset();

	union
	{
		struct {} empty;
		Resource resource;
	} storage;
};

struct OptionalSharedResource
{
	OptionalSharedResource();
	OptionalSharedResource(void* res);
	OptionalSharedResource(Resource);
	OptionalSharedResource(SharedResrouce);
	OptionalSharedResource(const OptionalSharedResource&);
	OptionalSharedResource(OptionalSharedResource&&);
	~OptionalSharedResource();

	operator bool () const;

	union
	{
		struct {} empty;
		SharedResrouce resource;
	} storage;
};

// Implementation of the wrapper
struct create_resource
{
	using error = std::string;

	FLOW_ACTION const
	{
		void* resource;
		const auto err = create_resource_c(&resource);
		if (err != 0) FLOW_CONTINUE(std::format("some error: {}", err));

		FLOW_CONTINUE({ resource });
	}
};

// Usage example

void use_or_print_on_error()
{
	create_resource{} >> cppflow::composition::flow +
	[](create_resource::error err) { std::cout << "create resource failed" << err << std::endl; } +
	[](const Resource res)
	{
		DoSomethingWithResource(res);
	};
}

void optional_store_from_raw()
{
	OptionalResource resource = create_resource{} >> cppflow::composition::flow +
	[](create_resource::error) -> OptionalResource { return {}; } +
	[](void* const res) -> OptionalResource { return { res }; };
}

void shared_store_from_optional()
{
	OptionalSharedResource resource = create_resource{} >> cppflow::composition::flow +
	[](create_resource::error) -> OptionalSharedResource { return {}; } +
	[](OptionalResource res) -> OptionalSharedResource
	{
		if (ShouldStoreResource()) return { res.reset() };
		else return {};
	};
}

```

Let's iterate again over the issues that I had with the former implementation just to be sure that we cover everything:
* *Inability to pick the directly use the storage method of your choosing*: Here the client of the wrapper decides which storage type to create
* *error information support requires unnecessary storage*: because we are providing two different continuations for the successful and the failing flow, the only storage that is needed in runtime is the fitting storage to the correct state (`std::string` for the error case and `resource` for the successful state).
* *revalidating the state*: when the successful state continuation procedure is called we don't need to check the validity of the class, when its called we can be sure that we have a valid resource that can be used, and in error cases, we are free to forward the same error forward or to modify it

## So CPS is good just for Resource types creation?
Dude (or dudette)! it has other benefits other than just code correctness for resource creation, jump to the wiki and see what it has to offer! (First, let me write the wiki)

## Woof, your code seems a bit complicated, can you explain a bit about the implementation details?
The implementation guide is in the works right now I'll put a link here when I'm done

## Yo That looks great! I can't wait to start writing CPS Style!
Wait wait wait, there are some major practical issues with CPS please read the wiki before jumping to conclusions

## I saw your implementation and it looks a bit cumbersome cant it be simpler?
Well if you have any advice about how to make this code please let me know, also jump to the wiki page called "C++ features I want to see", there's a compilation of features that could make CPS simpler

## Can I buy you a coffee?
No. I buy my own coffee.