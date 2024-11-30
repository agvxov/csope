# NOTE: Kernel mode should be used accross the board,
#        because system headers are volatile.
#       Writting tests for them would be foolish.
#       They could even introduce interfering
#        symbols in the future by pure chance,
#        if not completely ignored.

# The following variables are magick numbers based on `dummy_project/`.
$f_definition_line = 5

class CMDTEST_misc_batch < Cmdtest::Testcase
  def test_no_arg
    cmd "csope" do
      stdout_equal /.+/
      stderr_equal /.+/
      exit_status 1
    end
  end
end

class CMDTEST_dummy_project < Cmdtest::Testcase
  def setup
    import_directory "test/dummy_project/", "./dummy_project/"
  end

  def test_find_f
    cmd "csope -k -L -0 f -s dummy_project/" do
      created_files ["cscope.out"]
      stdout_equal /\A(.*\n){2}\Z/
    end
  end

  def test_find_def_f
    cmd "csope -k -L -1 f -s dummy_project/" do
      created_files ["cscope.out"]
      stdout_equal /\A.+#{$f_definition_line}.+\n\Z/
    end
  end
end
