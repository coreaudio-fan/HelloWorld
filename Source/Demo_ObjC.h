#pragma once

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

// Block typedefs give complex block signatures a readable name.
// Without a typedef, block parameters in method signatures quickly become unreadable.
// Note: block properties must be declared 'copy' (not 'strong') so the block
// is copied from the stack to the heap when assigned.
typedef void	(^HWStudentBlock)(NSString *name, float score);
typedef BOOL	(^HWStudentPredicate)(NSString *name, float score);

// Class prefix 'HW' follows the Objective-C convention of prefixing type names
// to avoid collisions in the global namespace.
@interface HWGradeBook : NSObject

// Stored block — fired whenever a student is added
@property (nonatomic, copy, nullable) HWStudentBlock onStudentAdded;

- (void)addStudentWithId:(int32_t)studentId
                    name:(NSString *)name
                   score:(float)score;

- (nullable NSString *)nameOfStudentWithId:(int32_t)studentId;
- (float)averageScore;
- (void)enumerateStudentsUsingBlock:(HWStudentBlock)block;
- (NSArray<NSString *> *)namesOfStudentsMatchingPredicate:(HWStudentPredicate)predicate;

+ (instancetype)gradeBookWithStudents:(NSArray<NSDictionary<NSString *, id> *> *)students;

@end

NS_ASSUME_NONNULL_END

void	run_demo_objc(void);
