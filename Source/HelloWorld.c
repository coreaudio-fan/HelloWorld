#include "HelloWorld.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// ----------------------------------------------------------------------------
// Private constants and types
// ----------------------------------------------------------------------------

#define DEMO_C_MAX_STUDENTS	64
#define DEMO_C_NAME_LEN		64

// Error codes returned by functions that can fail
#define DEMO_C_OK		 0
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
typedef void		(*Student_Visitor)(const Student* in_student_ptr, void* io_context_ptr);
typedef int32_t		(*Student_Predicate)(const Student* in_student_ptr, void* in_context_ptr);

// ----------------------------------------------------------------------------
// Private storage
// ----------------------------------------------------------------------------

static Student	s_students[DEMO_C_MAX_STUDENTS];
static int32_t	s_count = 0;

// ----------------------------------------------------------------------------
// Private helpers
// ----------------------------------------------------------------------------

static int32_t	is_valid_score(float in_score)
{
	return (in_score >= 0.0f) && (in_score <= 100.0f);
}

static int32_t	find_index(int32_t in_id)
{
	int32_t found_index = -1;
	for (int32_t student_index = 0; student_index < s_count; student_index++)
	{
		if (s_students[student_index].m_id == in_id)
		{
			found_index = student_index;
			break;
		}
	}
	return found_index;
}

// Named callbacks used with foreach_student and filter_students.
// These are the C equivalent of closures: the context pointer carries
// any data the callback needs to "capture".

static void	accumulate_score(const Student* in_student_ptr, void* io_context_ptr)
{
	float* total_ptr = (float*)io_context_ptr;
	*total_ptr += in_student_ptr->m_score;
}

static int32_t	above_threshold(const Student* in_student_ptr, void* in_context_ptr)
{
	float threshold = *(const float*)in_context_ptr;
	return in_student_ptr->m_score >= threshold;
}

// ----------------------------------------------------------------------------
// Private API
// ----------------------------------------------------------------------------

static int32_t	add_student(int32_t in_id, const char* in_name, float in_score)
{
	if (s_count >= DEMO_C_MAX_STUDENTS)
	{
		return DEMO_C_ERR_FULL;
	}
	if ((in_name == NULL) || (in_name[0] == '\0'))
	{
		return DEMO_C_ERR_INVALID;
	}
	if (!is_valid_score(in_score))
	{
		return DEMO_C_ERR_INVALID;
	}
	if (find_index(in_id) >= 0)
	{
		return DEMO_C_ERR_INVALID;
	}

	Student* student_ptr = &s_students[s_count++];
	student_ptr->m_id		= in_id;
	student_ptr->m_score	= in_score;
	strncpy(student_ptr->m_name, in_name, DEMO_C_NAME_LEN - 1);
	student_ptr->m_name[DEMO_C_NAME_LEN - 1] = '\0';

	return DEMO_C_OK;
}

static const Student*	find_student(int32_t in_id)
{
	int32_t found_index = find_index(in_id);
	return (found_index >= 0) ? &s_students[found_index] : NULL;
}

static void	print_student(const Student* in_student_ptr)
{
	if (in_student_ptr == NULL)
	{
		return;
	}
	printf("  [%3d]  %-20s  %5.1f\n", in_student_ptr->m_id, in_student_ptr->m_name, in_student_ptr->m_score);
}

static void	print_all_students(void)
{
	printf("  [ id]  %-20s  score\n", "name");
	printf("  -----  --------------------  -----\n");
	for (int32_t student_index = 0; student_index < s_count; student_index++)
	{
		print_student(&s_students[student_index]);
	}
}

static float	average_score(void)
{
	if (s_count == 0)
	{
		return 0.0f;
	}

	float total = 0.0f;
	for (int32_t student_index = 0; student_index < s_count; student_index++)
	{
		total += s_students[student_index].m_score;
	}

	return total / (float)s_count;
}

static void	foreach_student(Student_Visitor in_visitor, void* io_context_ptr)
{
	for (int32_t student_index = 0; student_index < s_count; student_index++)
	{
		in_visitor(&s_students[student_index], io_context_ptr);
	}
}

static int32_t	filter_students(Student_Predicate in_predicate, void* in_context_ptr, Student* out_buffer, int32_t in_buffer_capacity)
{
	int32_t	out_count = 0;
	for (int32_t student_index = 0; (student_index < s_count) && (out_count < in_buffer_capacity); student_index++)
	{
		if (in_predicate(&s_students[student_index], in_context_ptr))
		{
			out_buffer[out_count++] = s_students[student_index];
		}
	}
	return out_count;
}

// ----------------------------------------------------------------------------
// Demo
// ----------------------------------------------------------------------------

void	run_demo_c(void)
{
	printf("\n=== C Demo ===\n\n");

	add_student(1, "Alice",	92.5f);
	add_student(2, "Bob",	78.0f);
	add_student(3, "Carol",	85.5f);
	add_student(4, "Dave",	61.0f);
	add_student(5, "Eve",	97.0f);

	printf("All students:\n");
	print_all_students();

	printf("\nAverage: %.1f\n", average_score());

	// foreach_student with a named callback and a context pointer —
	// the C equivalent of a closure capturing a local variable
	float total = 0.0f;
	foreach_student(accumulate_score, &total);
	printf("Total (via visitor callback): %.1f\n", total);

	// filter_students with a predicate and a context pointer
	float		threshold = 80.0f;
	Student		high_achievers[DEMO_C_MAX_STUDENTS];
	int32_t		high_achiever_count = filter_students(above_threshold, &threshold, high_achievers, DEMO_C_MAX_STUDENTS);

	printf("\nStudents scoring >= %.0f:\n", threshold);
	for (int32_t achiever_index = 0; achiever_index < high_achiever_count; achiever_index++)
	{
		print_student(&high_achievers[achiever_index]);
	}

	// find_student: returns a pointer or NULL
	const Student*	found_ptr = find_student(3);
	if (found_ptr != NULL)
	{
		printf("\nFound student 3: %s\n", found_ptr->m_name);
	}

	if (find_student(99) == NULL)
	{
		printf("Student 99: not found\n");
	}
}

// ----------------------------------------------------------------------------
// Hello
// ----------------------------------------------------------------------------

void	hello_c(void)
{
	puts("Hello from C");
}
