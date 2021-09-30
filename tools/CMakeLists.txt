
add_executable(exact exact.c
   fasta.h fasta.c fastq.h fastq.c sam.h sam.c)
target_link_libraries(exact cstr)
set_target_properties(
   exact PROPERTIES FOLDER Tools
)
