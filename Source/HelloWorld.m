#import <Foundation/Foundation.h>
#include "HelloWorld_objc.h"
#include <stdio.h>

NS_ASSUME_NONNULL_BEGIN

// Block typedefs give complex block signatures a readable name.
// Without a typedef, block parameters in method signatures quickly become unreadable.
// Note: block properties must be declared 'copy' (not 'strong') so the block
// is copied from the stack to the heap when assigned.
typedef void	(^HWStudentBlock)(NSString* in_name, float in_score);
typedef BOOL	(^HWStudentPredicate)(NSString* in_name, float in_score);

// Private student model — used only inside this file
@interface HWStudent : NSObject
@property (nonatomic, assign)	int32_t		studentId;
@property (nonatomic, copy)		NSString*	name;
@property (nonatomic, assign)	float		score;
- (instancetype)initWithId:(int32_t)in_studentId name:(NSString*)in_name score:(float)in_score;
@end

@implementation HWStudent

@synthesize studentId	= m_studentId;
@synthesize name		= m_name;
@synthesize score		= m_score;

- (instancetype)initWithId:(int32_t)in_studentId name:(NSString*)in_name score:(float)in_score
{
	self = [super init];
	if (self)
	{
		m_studentId	= in_studentId;
		m_name		= [in_name copy];
		m_score		= in_score;
	}
	return self;
}

@end

// Class prefix 'HW' follows the Objective-C convention of prefixing type names
// to avoid collisions in the global namespace.
@interface HWGradeBook : NSObject
@property (nonatomic, copy, nullable)	HWStudentBlock				onStudentAdded;
@property (nonatomic, strong)			NSMutableArray<HWStudent*>*	students;

- (void)addStudentWithId:(int32_t)in_studentId name:(NSString*)in_name score:(float)in_score;

- (nullable NSString*)nameOfStudentWithId:(int32_t)in_studentId;
- (float)averageScore;
- (void)enumerateStudentsUsingBlock:(HWStudentBlock)in_block;
- (NSArray<NSString*>*)namesOfStudentsMatchingPredicate:(HWStudentPredicate)in_predicate;

+ (instancetype)gradeBookWithStudents:(NSArray<NSDictionary<NSString*, id>*>*)in_studentDictionaries;
@end

@implementation HWGradeBook

@synthesize onStudentAdded	= m_onStudentAdded;
@synthesize students		= m_students;

- (instancetype)init
{
	self = [super init];
	if (self)
	{
		m_students = [NSMutableArray array];
	}
	return self;
}

+ (instancetype)gradeBookWithStudents:(NSArray<NSDictionary<NSString*, id>*>*)in_studentDictionaries
{
	HWGradeBook* book = [[HWGradeBook alloc] init];
	for (NSDictionary* studentRecord in in_studentDictionaries)
	{
		[book addStudentWithId:[studentRecord[@"id"] intValue] name:studentRecord[@"name"] score:[studentRecord[@"score"] floatValue]];
	}
	return book;
}

- (void)addStudentWithId:(int32_t)in_studentId name:(NSString*)in_name score:(float)in_score
{
	HWStudent* student = [[HWStudent alloc] initWithId:in_studentId name:in_name score:in_score];
	[self.students addObject:student];
	if (self.onStudentAdded)
	{
		self.onStudentAdded(in_name, in_score);
	}
}

- (nullable NSString*)nameOfStudentWithId:(int32_t)in_studentId
{
	NSString* foundName = nil;
	for (HWStudent* student in self.students)
	{
		if (student.studentId == in_studentId)
		{
			foundName = student.name;
			break;
		}
	}
	return foundName;
}

- (float)averageScore
{
	if (self.students.count == 0)
	{
		return 0.0f;
	}
	float total = 0.0f;
	for (HWStudent* student in self.students)
	{
		total += student.score;
	}
	return total / (float)self.students.count;
}

- (void)enumerateStudentsUsingBlock:(HWStudentBlock)in_block
{
	for (HWStudent* student in self.students)
	{
		in_block(student.name, student.score);
	}
}

- (NSArray<NSString*>*)namesOfStudentsMatchingPredicate:(HWStudentPredicate)in_predicate
{
	NSMutableArray<NSString*>* matchingNames = [NSMutableArray array];
	for (HWStudent* student in self.students)
	{
		if (in_predicate(student.name, student.score))
		{
			[matchingNames addObject:student.name];
		}
	}
	return [matchingNames copy];
}

@end

NS_ASSUME_NONNULL_END

// -----------------------------------------------------------------------
// Demo
// -----------------------------------------------------------------------
void	run_demo_objc(void)
{
	printf("\n=== Objective-C Demo ===\n\n");

	HWGradeBook* book = [[HWGradeBook alloc] init];

	// Stored block: captures no external state here, but illustrates the pattern.
	// The property is declared 'copy' so the block is heap-allocated.
	book.onStudentAdded =	^(NSString* in_name, float in_score)
							{
								printf("  + Added: %s (%.1f)\n", in_name.UTF8String, in_score);
							};

	[book addStudentWithId:1 name:@"Alice"	score:92.5f];
	[book addStudentWithId:2 name:@"Bob"	score:78.0f];
	[book addStudentWithId:3 name:@"Carol"	score:85.5f];
	[book addStudentWithId:4 name:@"Dave"	score:61.0f];
	[book addStudentWithId:5 name:@"Eve"	score:97.0f];

	printf("\nAll students:\n");
	[book enumerateStudentsUsingBlock:	^(NSString* in_name, float in_score)
										{
											printf("  %-20s  %.1f\n", in_name.UTF8String, in_score);
										}];

	printf("\nAverage: %.1f\n", book.averageScore);

	// Filter with a predicate block — inline block as trailing argument
	NSArray<NSString*>* highAchievers = [book namesOfStudentsMatchingPredicate:	^BOOL(NSString* in_name, float in_score)
																				{
																					return (in_name.length > 0) && (in_score >= 80.0f);
																				}];
	printf("\nStudents scoring >= 80:\n");
	for (NSString* name in highAchievers)
	{
		printf("  %s\n", name.UTF8String);
	}

	// __weak/__strong dance: prevents a retain cycle when a block captures an
	// object that owns the block. Without __weak, each would keep the other alive.
	__weak HWGradeBook* weakBook = book;
	HWStudentBlock snapshotBlock =	^(NSString* in_name, float in_score)
									{
										__strong HWGradeBook* strongBook = weakBook;
										if (strongBook != nil)
										{
											printf("  [snapshot] %-20s %.1f  (avg: %.1f)\n", in_name.UTF8String, in_score, strongBook.averageScore);
										}
									};
	printf("\nEnumerating with weak/strong capture pattern:\n");
	[book enumerateStudentsUsingBlock:snapshotBlock];

	// Dot notation vs message syntax — both compile; style guides disagree.
	// Apple's own code mixes both; many teams pick one and enforce it.
	float averageViaDot		= book.averageScore; // dot notation (property access style)
	float averageViaMessage	= [book averageScore]; // message syntax (traditional Objective-C)
	printf("\navg (dot): %.1f   avg (message): %.1f\n", averageViaDot, averageViaMessage);

	// Factory class method
	HWGradeBook* factoryBook = [HWGradeBook gradeBookWithStudents:@[
		@{@"id": @10, @"name": @"Frank", @"score": @88.0},
		@{@"id": @11, @"name": @"Grace", @"score": @74.5},
	]];
	printf("\nBook2 average: %.1f\n", factoryBook.averageScore);
}

// -----------------------------------------------------------------------
// Hello
// -----------------------------------------------------------------------
void	hello_objc(void)
{
	NSString* message = @"Hello from Objective-C";
	fprintf(stdout, "%s\n", message.UTF8String);
}
