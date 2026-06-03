#include "Demo_C.h"
#include <stdio.h>
#include <string.h>

static Student	s_students[DEMO_C_MAX_STUDENTS];
static int32_t	s_count = 0;

// ----------------------------------------------------------------------------
// Internal helpers (static = file-local linkage)
// ----------------------------------------------------------------------------

static int32_t	is_valid_score(float score)
{
	return score >= 0.0f && score <= 100.0f;
}

static int32_t	find_index(int32_t id)
{
	for (int32_t i = 0; i < s_count; i++)
	{
		if (s_students[i].m_id == id)
			return i;
	}
	return -1;
}

// Named callbacks used with foreach_student and filter_students.
// These are the C equivalent of closures: the context pointer carries
// any data the callback needs to "capture".

static void	accumulate_score(const Student *student, void *context)
{
	float *total = (float *)context;
	*total += student->m_score;
}

static int32_t	above_threshold(const Student *student, void *context)
{
	float threshold = *(const float *)context;
	return student->m_score >= threshold;
}

// ----------------------------------------------------------------------------
// Public API
// ----------------------------------------------------------------------------

int32_t	add_student(int32_t id, const char *name, float score)
{
	if (s_count >= DEMO_C_MAX_STUDENTS)
		return DEMO_C_ERR_FULL;
	if (name == NULL || name[0] == '\0')
		return DEMO_C_ERR_INVALID;
	if (!is_valid_score(score))
		return DEMO_C_ERR_INVALID;
	if (find_index(id) >= 0)
		return DEMO_C_ERR_INVALID;

	Student *s = &s_students[s_count++];
	s->m_id    = id;
	s->m_score = score;
	strncpy(s->m_name, name, DEMO_C_NAME_LEN - 1);
	s->m_name[DEMO_C_NAME_LEN - 1] = '\0';

	return DEMO_C_OK;
}

const Student	*find_student(int32_t id)
{
	int32_t index = find_index(id);
	return (index >= 0) ? &s_students[index] : NULL;
}

void	print_student(const Student *student)
{
	if (student == NULL)
		return;
	printf("  [%3d]  %-20s  %5.1f\n", student->m_id, student->m_name, student->m_score);
}

void	print_all_students(void)
{
	printf("  [ id]  %-20s  score\n", "name");
	printf("  -----  --------------------  -----\n");
	for (int32_t i = 0; i < s_count; i++)
		print_student(&s_students[i]);
}

float	average_score(void)
{
	if (s_count == 0)
		return 0.0f;

	float total = 0.0f;
	for (int32_t i = 0; i < s_count; i++)
		total += s_students[i].m_score;

	return total / (float)s_count;
}

void	foreach_student(Student_Visitor visitor, void *context)
{
	for (int32_t i = 0; i < s_count; i++)
		visitor(&s_students[i], context);
}

int32_t	filter_students(Student_Predicate predicate, void *context,
		Student *out_buf, int32_t buf_size)
{
	int32_t	out_count = 0;
	for (int32_t i = 0; i < s_count && out_count < buf_size; i++)
	{
		if (predicate(&s_students[i], context))
			out_buf[out_count++] = s_students[i];
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
	int32_t		count = filter_students(above_threshold, &threshold,
				high_achievers, DEMO_C_MAX_STUDENTS);

	printf("\nStudents scoring >= %.0f:\n", threshold);
	for (int32_t i = 0; i < count; i++)
		print_student(&high_achievers[i]);

	// find_student: returns a pointer or NULL
	const Student	*found = find_student(3);
	if (found != NULL)
		printf("\nFound student 3: %s\n", found->m_name);

	if (find_student(99) == NULL)
		printf("Student 99: not found\n");
}
