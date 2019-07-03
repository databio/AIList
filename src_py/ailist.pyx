from libc.stdint cimport int64_t, int32_t, uint32_t
from libc.stdlib cimport free

cdef extern from "AIList.h":

	ctypedef struct gdata_t:
		pass

	ctypedef struct ailist_t:
		pass

	ailist_t *ailist_init()
	void ailist_destroy(ailist_t *ail)
	void ailist_add(ailist_t *ail, const char *ctg, uint32_t s, uint32_t e, uint32_t v)
	void ailist_construct(ailist_t *ail, int cLen)
	uint32_t ailist_query(ailist_t *ail, char *ctg, uint32_t qs, uint32_t qe,  int32_t *gid, uint32_t *mr, uint32_t **ir) 
	uint32_t get_start(const ailist_t *ail, int32_t gid, uint32_t i)
	uint32_t get_end(const ailist_t *ail, int32_t gid, uint32_t i)
	int32_t get_value(const ailist_t *ail, int32_t gid, uint32_t i)	

cdef class ailist:
	cdef ailist_t *ail

	def __cinit__(self):
		self.ail = ailist_init()

	def __dealloc__(self):
		ailist_destroy(self.ail)

	def add(self, ctg, s, e, v=0):
		ailist_add(self.ail, str.encode(ctg), s, e, v)

	def construct(self):
		ailist_construct(self.ail, 20)

	def query(self, ctg, qs, qe):
		cdef uint32_t *ir = NULL
		cdef uint32_t mr = 0		
		cdef uint32_t n
		cdef int32_t gid
		n = ailist_query(self.ail, str.encode(ctg), qs, qe, &gid, &mr, &ir)
		free(ir)
		return n
		
