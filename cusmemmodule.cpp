#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <cstdlib>
#include <iostream>
#include <memory_resource>
#include <string>

static void* my_alloc(void *ctx, size_t size);
static void* my_realloc(void *ctx, void *ptr, size_t new_size);
static void* my_calloc(void *ctx, size_t nelem, size_t elsize);
static void my_free(void *ctx, void *ptr);


std::pmr::unsynchronized_pool_resource pool{};

const int META_BLOC_SIZE = 8;

static void* my_alloc(void *ctx, size_t size)
{
    printf("my_alloc\n");
	if(size == 0)
		size = 1;

    size += META_BLOC_SIZE;
		
	if(size > 120)
    {
        auto addr = (unsigned short *)malloc(size);
        std::cout << "malloc ( ): " << addr << std::endl;
        *addr = 0;
        std::cout << "allocated (raw) (" << size << "): " << (addr + META_BLOC_SIZE/2) << std::endl;
        return (void *) (addr + META_BLOC_SIZE/2);
    }
	else
	{
		printf("using pool with %d\n", size);
		auto addr = (unsigned short*)pool.allocate(size);
        *addr = size;
        std::cout << "allocated: " << addr << std::endl;
        return (void *) (addr + META_BLOC_SIZE/2);
	}

}

static void* my_realloc(void *ctx, void *ptr, size_t new_size)
{
    printf("my_realloc %d\n", new_size);

    bool from_pool = *((unsigned short*)ptr - META_BLOC_SIZE/2) != 0;

    if(!from_pool){
        std::cout << "real pool ! : " << ptr << std::endl;
        new_size += META_BLOC_SIZE;

        std::cout << "realloc ( ): " << (unsigned short*)ptr - META_BLOC_SIZE/2 << std::endl;
        auto addr = (unsigned short *)realloc((unsigned short*)ptr - META_BLOC_SIZE/2, new_size);
        *addr = 0;
        return (void *) (addr + META_BLOC_SIZE/2);
    }
    else {
        auto addr = my_alloc(ctx, new_size);
        memcpy(addr, ptr, *((unsigned short *) ptr - META_BLOC_SIZE/2));

        my_free(ctx, ptr);

        return addr;
    }
}

static void* my_calloc(void *ctx, size_t nelem, size_t elsize)
{
    printf("my_calloc\n");
	if (nelem == 0 || elsize == 0) {
        nelem = 1;
        elsize = 1;
    }

    auto addr = my_alloc(ctx, nelem * elsize);

    memset(addr, 0, nelem * elsize);

    return addr;
}

static void my_free(void *ctx, void *ptr)
{
    printf("my_free\n");
    bool from_pool = *((unsigned short*)ptr - META_BLOC_SIZE/2) != 0;

    if (from_pool) {
        printf("Dallocate from pool\n");
        std::cout << "freeing: " << (unsigned short *)ptr - META_BLOC_SIZE/2 << " with size: " << *((unsigned short *) ptr - META_BLOC_SIZE/2) << std::endl;
        pool.deallocate((unsigned short *) ptr - META_BLOC_SIZE/2, *((unsigned short *) ptr - META_BLOC_SIZE/2));
    } else {
        free((unsigned short *) ptr - META_BLOC_SIZE/2);
    }
}

static PyObject* setup_alloc(PyObject *self, PyObject *args)
{
	PyMemAllocatorEx alloc;

	size_t tmp = 2;

	alloc.ctx = &tmp;
	alloc.malloc = my_alloc;
	alloc.calloc = my_calloc;
	alloc.realloc = my_realloc;
	alloc.free = my_free;

	std::cout << "test" << std::endl;

	PyMem_SetAllocator(PYMEM_DOMAIN_OBJ, &alloc);
    PyMem_SetAllocator(PYMEM_DOMAIN_MEM, &alloc);

	Py_RETURN_NONE;
}	

static PyMethodDef CusMemMethods[] = {
	{"setup_alloc", setup_alloc, METH_VARARGS, "Setup a custom allocator"},
	{NULL, NULL, 0, NULL}
};

static struct PyModuleDef cusmemmodule = {
	PyModuleDef_HEAD_INIT,
	"cusmem",
	NULL,
	-1,
	CusMemMethods
};

PyMODINIT_FUNC PyInit_cusmem()
{
	return PyModule_Create(&cusmemmodule);
}
