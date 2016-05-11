
#include "containers/containers.h"

extern const char *get_container_name(uint8_t typecode);

extern int container_get_cardinality(const void *container, uint8_t typecode);

extern void *container_iand(void *c1, uint8_t type1, const void *c2,
                            uint8_t type2, uint8_t *result_type);

extern void *container_ior(void *c1, uint8_t type1, const void *c2,
                           uint8_t type2, uint8_t *result_type);

void container_printf(const void *container, uint8_t typecode) {
	container = container_unwrap_shared(container,&typecode);
	switch (typecode) {
        case BITSET_CONTAINER_TYPE_CODE:
            bitset_container_printf(container);
            return;
        case ARRAY_CONTAINER_TYPE_CODE:
            array_container_printf(container);
            return;
        case RUN_CONTAINER_TYPE_CODE:
            run_container_printf(container);
            return;
        default:
            __builtin_unreachable();
    }
}

void container_printf_as_uint32_array(const void *container, uint8_t typecode,
                                      uint32_t base) {
	container = container_unwrap_shared(container,&typecode);
	switch (typecode) {
        case BITSET_CONTAINER_TYPE_CODE:
            bitset_container_printf_as_uint32_array(container, base);
            return;
        case ARRAY_CONTAINER_TYPE_CODE:
            array_container_printf_as_uint32_array(container, base);
            return;
        case RUN_CONTAINER_TYPE_CODE:
            run_container_printf_as_uint32_array(container, base);
            return;
            return;
        default:
            __builtin_unreachable();
    }
}

int32_t container_serialize(const void *container, uint8_t typecode, char *buf) {
	container = container_unwrap_shared(container,&typecode);
	switch (typecode) {
        case BITSET_CONTAINER_TYPE_CODE:
            return (bitset_container_serialize((bitset_container_t *)container,
                                               buf));
        case ARRAY_CONTAINER_TYPE_CODE:
            return (
                array_container_serialize((array_container_t *)container, buf));
        case RUN_CONTAINER_TYPE_CODE:
            return (run_container_serialize((run_container_t *)container, buf));
        default:
            assert(0);
            __builtin_unreachable();
            return (-1);
    }
}

uint32_t container_serialization_len(const void *container, uint8_t typecode) {
	container = container_unwrap_shared(container,&typecode);
    switch (typecode) {
        case BITSET_CONTAINER_TYPE_CODE:
            return bitset_container_serialization_len();
        case ARRAY_CONTAINER_TYPE_CODE:
            return array_container_serialization_len(
                (array_container_t *)container);
        case RUN_CONTAINER_TYPE_CODE:
            return run_container_serialization_len(
                (run_container_t *)container);
        default:
            assert(0);
            __builtin_unreachable();
            return (0);
    }
}

void *container_deserialize(uint8_t typecode, const char *buf, size_t buf_len) {
    switch (typecode) {
        case BITSET_CONTAINER_TYPE_CODE:
            return (bitset_container_deserialize(buf, buf_len));
        case ARRAY_CONTAINER_TYPE_CODE:
            return (array_container_deserialize(buf, buf_len));
        case RUN_CONTAINER_TYPE_CODE:
            return (run_container_deserialize(buf, buf_len));
        case SHARED_CONTAINER_TYPE_CODE:
        	printf("this should never happen.\n");
			assert(0);
            __builtin_unreachable();
            return (NULL);
        default:
            assert(0);
            __builtin_unreachable();
            return (NULL);
    }
}

extern bool container_nonzero_cardinality(const void *container,
                                          uint8_t typecode);

extern void container_free(void *container, uint8_t typecode);

extern int container_to_uint32_array(uint32_t *output, const void *container,
                                     uint8_t typecode, uint32_t base);

extern void *container_add(void *container, uint16_t val, uint8_t typecode,
                           uint8_t *new_typecode);

extern bool container_contains(const void *container, uint16_t val,
                               uint8_t typecode);

extern void *container_clone(const void *container, uint8_t typecode);

extern void *container_and(const void *c1, uint8_t type1, const void *c2,
                           uint8_t type2, uint8_t *result_type);

extern void *container_or(const void *c1, uint8_t type1, const void *c2,
                          uint8_t type2, uint8_t *result_type);

void *get_copy_of_container(void * container, uint8_t * typecode, bool copy_on_write) {
  if(copy_on_write) {
	shared_container_t *shared_container;
	if(*typecode == SHARED_CONTAINER_TYPE_CODE) {
		shared_container = (shared_container_t *) container;
		shared_container->counter += 1;
		return shared_container;
	}
	assert(*typecode != SHARED_CONTAINER_TYPE_CODE);

    if ((shared_container = malloc(sizeof(shared_container_t))) == NULL) {
        return NULL;
    }

    shared_container->container = container;
    shared_container->typecode = *typecode;

    shared_container->counter = 2;
    *typecode = SHARED_CONTAINER_TYPE_CODE;

    return shared_container;
  }//copy_on_write
  // otherwise, no copy on write...
  const void * actualcontainer = container_unwrap_shared((const void *)container, typecode);
  assert(*typecode != SHARED_CONTAINER_TYPE_CODE);
  return container_clone(actualcontainer, *typecode);

}
/**
 * Copies a container, requires a typecode. This allocates new memory, caller
 * is responsible for deallocation.
 */
void *container_clone(const void *container, uint8_t typecode) {
  	container = container_unwrap_shared(container,&typecode);
    switch (typecode) {
        case BITSET_CONTAINER_TYPE_CODE:
            return bitset_container_clone((bitset_container_t *)container);
        case ARRAY_CONTAINER_TYPE_CODE:
            return array_container_clone((array_container_t *)container);
        case RUN_CONTAINER_TYPE_CODE:
            return run_container_clone((run_container_t *)container);
        case SHARED_CONTAINER_TYPE_CODE:
        	printf("shared containers are not cloneable\n");
            assert(false);
            return NULL;
        default:
            assert(false);
            __builtin_unreachable();
            return NULL;
    }
}

void * shared_container_extract_copy (shared_container_t * container, uint8_t * typecode) {
	assert(container->counter > 0);
	assert(container->typecode != SHARED_CONTAINER_TYPE_CODE);
	container->counter--;
        *typecode = container->typecode;
        void * answer;
	if(container->counter == 0) {
                answer = container->container;
		container->container = NULL; // paranoid
		free(container);
	} else {
                answer = container_clone(container->container, *typecode);
        }
       	assert(*typecode != SHARED_CONTAINER_TYPE_CODE);
        return answer;
}

void shared_container_free (shared_container_t * container) {
	assert(container->counter > 0);
	container->counter--;
	if(container->counter == 0) {
		assert(container->typecode != SHARED_CONTAINER_TYPE_CODE);
		container_free(container->container,container->typecode);
		container->container = NULL; // paranoid
		free(container);
	}
}
