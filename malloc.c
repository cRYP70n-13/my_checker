/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isaadi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/12/19 21:15:17 by isaadi            #+#    #+#             */
/*   Updated: 2019/12/19 21:15:19 by isaadi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <stdio.h>
#include <execinfo.h>

typedef struct	s_blk
{
	void			*addr;
	size_t			bytes;
	void			*callstack[128];
	char			**trace;
	size_t			size;
	struct s_blk	*next;
}				t_blk;

static t_blk	*_x;
static size_t	m_count, f_count;

void	*xmalloc(size_t xbytes)
{
	void			*rtn;
	t_blk			*tmp;
	size_t			size;

	if (!xbytes)
		return (NULL);
	if (!(rtn = malloc(xbytes)))
		return (NULL);
	if (!_x)
	{
		if (!(_x = (t_blk*)malloc(sizeof(t_blk))))
		{
			printf("xmalloc failed.\n");
			return (NULL);
		}
		_x->size = backtrace(_x->callstack, 128);
		_x->trace = backtrace_symbols(_x->callstack, _x->size);
		_x->addr = rtn;
		_x->bytes = xbytes;
		_x->next = NULL;
	}
	else
	{
		tmp = _x;
		while (tmp->next)
			tmp = tmp->next;
		if (!(tmp->next = (t_blk*)malloc(sizeof(t_blk))))
		{
			printf("xmalloc failed.\n");
			while (tmp)
			{
				tmp = _x->next;
				free(_x);
				_x = tmp;
			}
			return (NULL);
		}
		tmp = tmp->next;
		tmp->size = backtrace(tmp->callstack, 128);
		tmp->trace = backtrace_symbols(tmp->callstack, tmp->size);
		tmp->addr = rtn;
		tmp->bytes = xbytes;
		tmp->next = NULL;
	}
	m_count++;
	return (rtn);
}

void	xfree(void *adr)
{
	t_blk *tmp;
	t_blk *tm;
	t_blk *t;

	tmp = _x;
	if (tmp && adr)
	{
		if (tmp->addr == adr)
		{
			_x = tmp->next;
			free(tmp->trace);
			free(tmp);
			free(adr);
			f_count++;
		}
		else
		{
			while (tmp && tmp->addr != adr)
			{
				tm = tmp;
				tmp = tmp->next;
			}
			if (tmp && adr == tmp->addr)
			{
				t = tmp->next;
				tm->next = t;
				free(tmp->trace);
				free(tmp);
				f_count++;
				free(adr);
			}
		}
	}
	else
	{
		if (!adr)
			free(adr);
		else
		{
			printf("\033[0;31mattempting to free non-user-allocated pointer. Good luck.\033[0m\n");
			free(adr);
		}
	}
}

void	leakcheck()
{
	t_blk	*tmp;
	int		i, l;
	size_t	x, j, k;

	k = 0;
	j = 0;
	i = 5;
	l = 0;
	tmp = _x;
	while (tmp && i)
	{
		printf("-----------------------------------\n");
		printf("address: %p, bytes: %zu\n", tmp->addr, tmp->bytes);
		x = 0;
		printf("traceback:\n");
		while (x < tmp->size)
			printf("%s\n", tmp->trace[x++]);
		k += tmp->bytes;
		tmp = tmp->next;
		l++;
		i--;
	}
	if (tmp)
		while (tmp)
		{
			k += tmp->bytes;
			tmp = tmp->next;
			j++;
			l++;
		}
	if (j)
		printf("-----------------------------------\nand %zu more unfreed allocation%s...\n", j, (j - 1 ? "s" : ""));
	printf("-----------------------------------\n");
	if (!k)
		printf("\033[0;32m");
	else if (k)
		printf("\033[0;31m");
	printf("%zu total allocation%s, %zu free%s, %zu byte%s lost in %d block%s.\033[0m\n", m_count, ((int)m_count - 1 ? "s" : "")
	, f_count, ((int)f_count - 1 ? "s" : ""), k, ((int)k - 1 ? "s" : ""), l, ((int)l - 1 ? "s" : ""));
	tmp = _x;
	while (tmp)
	{
		_x = _x->next;
		free(tmp->trace);
		free(tmp);
		tmp = _x;
	}
}

void	leakcheckfull()
{
	t_blk	*tmp;
	size_t	x, j, k;
	int		i;

	k = 0;
	j = 0;
	i = 0;
	tmp = _x;
	while (tmp)
	{
		printf("-----------------------------------\n");
		printf("address: %p, bytes: %zu\n", tmp->addr, tmp->bytes);
		x = 0;
		printf("traceback:\n");
		while (x < tmp->size)
			printf("%s\n", tmp->trace[x++]);
		k += tmp->bytes;
		i++;
		tmp = tmp->next;
	}
	printf("-----------------------------------\n");
	if (!k)
		printf("\033[0;32m");
	else if (k)
		printf("\033[0;31m");
	printf("%zu total allocation%s, %zu free%s, %zu byte%s lost in %d block%s.\033[0m\n", m_count, ((int)m_count - 1 ? "s" : "")
	, f_count, ((int)f_count - 1 ? "s" : ""), k, ((int)k - 1 ? "s" : ""), i, ((int)i - 1 ? "s" : ""));
	tmp = _x;
	while (tmp)
	{
		_x = _x->next;
		free(tmp->trace);
		free(tmp);
		tmp = _x;
	}
}