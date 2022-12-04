; ModuleID = 'pac_example.c'
source_filename = "pac_example.c"
target datalayout = "e-m:e-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128"
target triple = "aarch64-pc-linux"

@buff = dso_local global [1024 x i8] zeroinitializer, align 512
@buff_ptr = dso_local local_unnamed_addr global ptr getelementptr inbounds ([1024 x i8], ptr @buff, i64 0, i64 512), align 8
@.str = private unnamed_addr constant [25 x i8] c"signing ptr %p with %lx\0A\00", align 1
@.str.1 = private unnamed_addr constant [14 x i8] c"paced_ptr %p\0A\00", align 1

; Function Attrs: nounwind uwtable
define dso_local ptr @pac(ptr noundef %0, i64 noundef %1) local_unnamed_addr #0 {
  %3 = getelementptr inbounds i64, ptr %0, i64 -1
  store i64 563, ptr %3, align 8, !tbaa !6
  %4 = shl i64 %1, 56
  %5 = ptrtoint ptr %0 to i64
  %6 = add i64 %4, %5
  %7 = inttoptr i64 %6 to ptr
  %8 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull @.str, ptr noundef %7, i64 noundef 563)
  %9 = tail call ptr asm "pacda $0, $2", "=r,0,r"(ptr %7, i64 563) #3, !srcloc !10
  %10 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull @.str.1, ptr noundef %9)
  ret ptr %9
}

; Function Attrs: nofree nounwind
declare noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #1

; Function Attrs: nofree nounwind readonly uwtable
define dso_local ptr @auth(ptr noundef %0) local_unnamed_addr #2 {
  %2 = ptrtoint ptr %0 to i64
  %3 = and i64 %2, 281474976710655
  %4 = inttoptr i64 %3 to ptr ; this is the real value of ptr
  %5 = lshr i64 %2, 56
  %6 = shl nsw i64 -1, %5
  %7 = and i64 %6, %2
  %8 = inttoptr i64 %7 to ptr
  %9 = and i64 %7, 281474976710655
  %10 = inttoptr i64 %9 to ptr
  %11 = getelementptr inbounds i64, ptr %10, i64 -1
  %12 = load i64, ptr %11, align 8, !tbaa !6
  %13 = tail call ptr asm "autda $0, $2", "=r,0,r"(ptr %8, i64 %12) #3, !srcloc !11
  ret ptr %4
}

; Function Attrs: nounwind uwtable
define dso_local i64 @main() local_unnamed_addr #0 {
  %1 = load ptr, ptr @buff_ptr, align 8, !tbaa !12
  %2 = getelementptr inbounds i64, ptr %1, i64 -1
  store i64 563, ptr %2, align 8, !tbaa !6
  %3 = ptrtoint ptr %1 to i64
  %4 = add i64 %3, 648518346341351424
  %5 = inttoptr i64 %4 to ptr
  %6 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull @.str, ptr noundef %5, i64 noundef 563)
  %7 = tail call ptr asm "pacda $0, $2", "=r,0,r"(ptr %5, i64 563) #3, !srcloc !10
  %8 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull @.str.1, ptr noundef %7)
  %9 = ptrtoint ptr %7 to i64
  %10 = and i64 %9, 281474976710655
  %11 = lshr i64 %9, 56
  %12 = shl nsw i64 -1, %11
  %13 = and i64 %12, %9
  %14 = inttoptr i64 %13 to ptr
  %15 = and i64 %13, 281474976710655
  %16 = inttoptr i64 %15 to ptr
  %17 = getelementptr inbounds i64, ptr %16, i64 -1
  %18 = load i64, ptr %17, align 8, !tbaa !6
  %19 = tail call ptr asm "autda $0, $2", "=r,0,r"(ptr %14, i64 %18) #3, !srcloc !11
  ret i64 %10
}

attributes #0 = { nounwind uwtable "frame-pointer"="non-leaf" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+neon,+pauth,+v8.2a" }
attributes #1 = { nofree nounwind "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+neon,+pauth,+v8.2a" }
attributes #2 = { nofree nounwind readonly uwtable "frame-pointer"="non-leaf" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+neon,+pauth,+v8.2a" }
attributes #3 = { nounwind readnone }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 1}
!5 = !{!"clang version 15.0.0 (git@github.com:mars-research/llvm-SFI.git b92d8eedd5547fb2713124c4aa462f9ba5b7acbc)"}
!6 = !{!7, !7, i64 0}
!7 = !{!"long", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{i64 658}
!11 = !{i64 1322}
!12 = !{!13, !13, i64 0}
!13 = !{!"any pointer", !8, i64 0}
