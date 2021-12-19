#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdlib.h>
#include <unistd.h>

static const size_t nb_gb = 3;

static struct MemoryBloc {
	size_t size;

	void *ptr_mem;
};

static struct MemoryNode {
	struct MemoryBloc node;
	struct MemoryBloc* next;
};

static struct MemOcean {
	struct MemoryNode* free_blocks;
	struct MemoryNode* reserved_blocks;
};

static struct MemOcean global_mem;

static void* my_malloc(void *ctx, size_t size)
{
	struct MemoryNode *iter = global_mem.free_blocks;
	struct MemoryNode *prev = NULL;

	while(iter != NULL && iter->node.size < size)
	{
		prev = iter;
		iter = iter->next;
	}	

	if(iter->size >= size)
	{
		// we remove first element
		if(!prev)
			global_mem.free_blocks = iter->next;
		else
			prev->next = iter->next;

		return iter->ptr_mem;
	}
}

static void* my_realloc(void *ctx, void *ptr, size_t new_size)
{
    int padding = *(int *)ctx;
	printf("memmr %d", padding);
    return realloc(ptr, new_size + padding);
}

static void my_free(void *ctx, void *ptr)
{
	printf("mmfree");
	fflush(stdout);
    //free(ptr);
}

static PyObject* setup_custom_allocator(PyObject *self, PyObject *args)
{
    PyMemAllocatorEx alloc;
    PyObjectArenaAllocator arena;

	// We allocate the necessary space
	global_mem.reserved_blocks = NULL;

	global_mem.free_blocks = (MemoryNode) malloc(sizeof(MemoryBlock));
	global_mem.free_blocks->node.size = nb_gb * 1024 * 1024 * 1024;
	global_mem.free_blocks->node.ptr_mem = malloc(global_mem.free_blocks->node.size);

    alloc.ctx = &global_mem;
    alloc.malloc = my_malloc;
    alloc.realloc = my_realloc;
    alloc.free = my_free;

    PyMem_SetAllocator(PYMEM_DOMAIN_RAW, &alloc);

	Py_RETURN_NONE;
}

static PyMethodDef MemAllocMethods[] = {
	{"setup_custom_allocator", setup_custom_allocator, METH_VARARGS, "Setup a custom memory allocator"}
};

static struct PyModuleDef memallocmodule = {
	PyModuleDef_HEAD_INIT,
	"mem_alloc",
	NULL,
	-1,
	MemAllocMethods
};

PyMODINIT_FUNC
PyInit_memalloc()
{
	return PyModule_Create(&memallocmodule);
}
