; ModuleID = 'test2.c'
source_filename = "test2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @maior(i32, i32) #0 !dbg !7 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  store i32 %0, i32* %4, align 4
  call void @llvm.dbg.declare(metadata i32* %4, metadata !11, metadata !DIExpression()), !dbg !12
  store i32 %1, i32* %5, align 4
  call void @llvm.dbg.declare(metadata i32* %5, metadata !13, metadata !DIExpression()), !dbg !14
  %6 = load i32, i32* %4, align 4, !dbg !15
  %7 = load i32, i32* %5, align 4, !dbg !17
  %8 = icmp slt i32 %6, %7, !dbg !18
  br i1 %8, label %9, label %11, !dbg !19

; <label>:9:                                      ; preds = %2
  %10 = load i32, i32* %5, align 4, !dbg !20
  store i32 %10, i32* %3, align 4, !dbg !22
  br label %15, !dbg !22

; <label>:11:                                     ; preds = %2
  %12 = load i32, i32* %4, align 4, !dbg !23
  %13 = load i32, i32* %5, align 4, !dbg !24
  %14 = add nsw i32 %12, %13, !dbg !25
  store i32 %14, i32* %3, align 4, !dbg !26
  br label %15, !dbg !26

; <label>:15:                                     ; preds = %11, %9
  %16 = load i32, i32* %3, align 4, !dbg !27
  ret i32 %16, !dbg !27
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 !dbg !28 {
  %1 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  %2 = call i32 @maior(i32 7, i32 5), !dbg !31
  ret i32 %2, !dbg !32
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 8.0.0 (Fedora 8.0.0-1.fc30)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "test2.c", directory: "/home/andreu/NetBeansProjects/Interpreter/tests")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"clang version 8.0.0 (Fedora 8.0.0-1.fc30)"}
!7 = distinct !DISubprogram(name: "maior", scope: !1, file: !1, line: 2, type: !8, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10, !10, !10}
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !DILocalVariable(name: "a", arg: 1, scope: !7, file: !1, line: 2, type: !10)
!12 = !DILocation(line: 2, column: 15, scope: !7)
!13 = !DILocalVariable(name: "b", arg: 2, scope: !7, file: !1, line: 2, type: !10)
!14 = !DILocation(line: 2, column: 22, scope: !7)
!15 = !DILocation(line: 3, column: 8, scope: !16)
!16 = distinct !DILexicalBlock(scope: !7, file: !1, line: 3, column: 8)
!17 = !DILocation(line: 3, column: 12, scope: !16)
!18 = !DILocation(line: 3, column: 10, scope: !16)
!19 = !DILocation(line: 3, column: 8, scope: !7)
!20 = !DILocation(line: 4, column: 16, scope: !21)
!21 = distinct !DILexicalBlock(scope: !16, file: !1, line: 3, column: 14)
!22 = !DILocation(line: 4, column: 9, scope: !21)
!23 = !DILocation(line: 6, column: 12, scope: !7)
!24 = !DILocation(line: 6, column: 14, scope: !7)
!25 = !DILocation(line: 6, column: 13, scope: !7)
!26 = !DILocation(line: 6, column: 5, scope: !7)
!27 = !DILocation(line: 7, column: 1, scope: !7)
!28 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 10, type: !29, scopeLine: 10, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!29 = !DISubroutineType(types: !30)
!30 = !{!10}
!31 = !DILocation(line: 13, column: 12, scope: !28)
!32 = !DILocation(line: 13, column: 5, scope: !28)
