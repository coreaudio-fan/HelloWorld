#pragma once

#include <stdint.h>

#define DEMO_C_MAX_STUDENTS	64
#define DEMO_C_NAME_LEN		64

// Error codes returned by functions that can fail
#define DEMO_C_OK		 	 0
#define DEMO_C_ERR_FULL		-1
#define DEMO_C_ERR_NOT_FOUND	-2
#define DEMO_C_ERR_INVALID	-3

typedef struct
{
	int32_t	m_id;
	char	m_name[DEMO_C_NAME_LEN];
	float	m_score;
} Student;

// The C idiom for closures: a function pointer paired with a caller-supplied
// context pointer that carries any "captured" data.
typedef void		(*Student_Visitor)(const Student *student, void *context);
typedef int32_t		(*Student_Predicate)(const Student *student, void *context);

int32_t			add_student(int32_t id, const char *name, float score);
const Student		*find_student(int32_t id);
void			print_student(const Student *student);
void			print_all_students(void);
float			average_score(void);
void			foreach_student(Student_Visitor visitor, void *context);
int32_t			filter_students(Student_Predicate predicate, void *context,
				Student *out_buf, int32_t buf_size);
void			run_demo_c(void);
