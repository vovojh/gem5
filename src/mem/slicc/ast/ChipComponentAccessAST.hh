
/*
 * Copyright (c) 1999-2008 Mark D. Hill and David A. Wood
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *
 *
 * Description:
 *
 * $Id: ChipComponentAccessAST.h 1.8 04/06/18 21:00:08-00:00 beckmann@cottons.cs.wisc.edu $
 *
 */

#ifndef ChipComponentAccessAST_H
#define ChipComponentAccessAST_H

#include "slicc_global.hh"
#include "StatementAST.hh"
#include "ExprAST.hh"
#include "VarExprAST.hh"
#include "TypeAST.hh"

class ChipComponentAccessAST : public ExprAST {
public:
  // Constructors

  // method call from local chip
  ChipComponentAccessAST(VarExprAST* machine, ExprAST* mach_version, VarExprAST* component, string* proc_name, Vector<ExprAST*>* expr_vec_ptr);
  // member access from local chip
  ChipComponentAccessAST(VarExprAST* machine, ExprAST* mach_version, VarExprAST* component, string* field_name);

  // method call from specified chip
  ChipComponentAccessAST(ExprAST* chip_version, VarExprAST* machine, ExprAST* mach_version, VarExprAST* component, string* proc_name, Vector<ExprAST*>* expr_vec_ptr);

  // member access from specified chip
  ChipComponentAccessAST(ExprAST* chip_version, VarExprAST* machine, ExprAST* mach_version, VarExprAST* component, string* field_name);

  // Destructor
  ~ChipComponentAccessAST();

  // Public Methods
  Type* generate(string& code) const;
  void findResources(Map<Var*, string>& resource_list) const;
  void print(ostream& out) const;
private:
  // Private Methods

  // Private copy constructor and assignment operator
  ChipComponentAccessAST(const ChipComponentAccessAST& obj);
  ChipComponentAccessAST& operator=(const ChipComponentAccessAST& obj);

  // Data Members (m_ prefix)
  VarExprAST* m_mach_var_ptr;
  VarExprAST* m_comp_var_ptr;
  ExprAST* m_mach_ver_expr_ptr;
  ExprAST* m_chip_ver_expr_ptr;
  Vector<ExprAST*>* m_expr_vec_ptr;
  string* m_proc_name_ptr;
  string* m_field_name_ptr;
};

// Output operator declaration
ostream& operator<<(ostream& out, const ChipComponentAccessAST& obj);

// ******************* Definitions *******************

// Output operator definition
extern inline
ostream& operator<<(ostream& out, const ChipComponentAccessAST& obj)
{
  obj.print(out);
  out << flush;
  return out;
}

#endif // ChipComponentAccessAST_H
