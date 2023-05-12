from simtestutil import SimTestLoader
import timer_16_extp

class TestCase(timer_16_extp.TestCase):
  
  _state_0 = "H"
  _state_1 = "L"
  
if __name__ == '__main__':
  
  from unittest import TextTestRunner
  tests = SimTestLoader("timer_16_extn_atmega128.elf").loadTestsFromTestCase(TestCase)
  TextTestRunner(verbosity = 2).run(tests)

# EOF