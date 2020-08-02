#define malloc xmalloc
#define free xfree
#include <stddef.h>
void	*xmalloc(size_t xbytes);
void	xfree(void *adr);
void	leakcheck();
void	leakcheckfull();
