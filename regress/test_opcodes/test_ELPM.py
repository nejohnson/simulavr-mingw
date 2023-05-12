#! /usr/bin/env python
###############################################################################
#
# simulavr - A simulator for the Atmel AVR family of microcontrollers.
# Copyright (C) 2001, 2002  Theodore A. Roth
# Copyright (C) 2015        Christian Taedcke
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
###############################################################################

"""Test the ELPM opcode.
"""

import base_test
from registers import Reg, SREG

class ELPM_TestFail(base_test.TestFail): pass

class base_ELPM(base_test.opcode_rampz_test):
	"""Generic test case for testing ELPM opcode.

	ELPM - Extended Load Program Memory

	Operation: R0 <- (RAMPZ:Z)

	opcode is '1001 0101 1101 1000'

	Only registers PC and R00 should be changed.
	"""

	def ensure_target_supports_opcode(self):
		if (not self.target.has_rampz):
			self.opcode_not_supported()

	def setup(self):
                # Set the register values
		self.setup_regs[Reg.R00] = 0
		self.setup_regs[Reg.R30] = (self.Z & 0xff)
		self.setup_regs[Reg.R31] = (self.Z >> 8)

		#setup RAMPZ register
		self.write_register_rampz(self.rampz & 0xff)

		# set up the val in memory
		self.prog_word_write(((self.rampz << 16) + self.Z) & 0xfffffe, 0xaa55 )

		# Return the raw opcode
		return 0x95D8

	def analyze_results(self):
		self.reg_changed.extend( [Reg.R00] )
		
		# check that result is correct
		if self.Z & 0x1:
			expect = 0xaa
		else:
			expect = 0x55

		got = self.anal_regs[Reg.R00]
		
		if expect != got:
			self.fail('ELPM: expect=%02x, got=%02x' % (expect, got))

#
# Template code for test case.
# The fail method will raise a test specific exception.
#
template = """
class ELPM_Z%04x_RZ%02x_TestFail(ELPM_TestFail): pass

class test_ELPM_Z%04x_RZ%02x(base_ELPM):
	Z = 0x%x
	rampz = 0x%x
	def fail(self,s):
		raise ELPM_Z%04x_RZ%02x_TestFail(s)
"""

#
# automagically generate the test_ELPM_* class definitions.
#
code = ''
for z in (0x10, 0x11, 0x100, 0x101):
        for rampz in (0x00, 0x01, 0x02):
                args = (z, rampz) * 4
                code += template % args
exec(code)
