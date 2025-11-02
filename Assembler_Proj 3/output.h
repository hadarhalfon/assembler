/*
 * output.h - Output file generation header
 * 
 * This header file declares functions for generating the three output files
 * produced by the assembler: .ob (object), .ent (entry), and .ext (external).
 */

#ifndef OUTPUT_H
#define OUTPUT_H

/* Output file generation functions */
void build_ext(Symbol* external_head,char *filename);           /* Generates external symbols file (.ext) */
void build_ent(Symbol* symbol_head,char *filename);             /* Generates entry symbols file (.ent) */
void build_ob(Word* Dword_head,Order* order_head, char *filename,int ICF,int DCF); /* Generates object file (.ob) */

#endif /* OUTPUT_H */