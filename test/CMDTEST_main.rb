# NOTE:
#   * Kernel mode should be used accross the board,
#      because system headers are volatile.
#     Writting tests for them would be foolish.
#     They could even introduce interference
#      symbols in the future by pure chance,
#      if not completely ignored.


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
  # The following variables are magick numbers based on `dummy_project/`.
  @@f_definition_line = 5
  @@f_mention_count   = 2

  def setup
    import_directory "test/dummy_project/", "./dummy_project/"
  end

  def test_do_not_explode
    cmd "csope -k -b -s dummy_project" do
      created_files ["cscope.out"]
      stderr_equal /.+/
      exit_status 0
    end
  end

  def test_find_def_f
    cmd "csope -k -L -1 f -s dummy_project/" do
      created_files ["cscope.out"]
      stderr_equal /.+/
      stdout_equal /\A.+#{@@f_definition_line}.+\n\Z/
    end
  end

  def test_find_f
    cmd "csope -k -L -0 f -s dummy_project/" do
      created_files ["cscope.out"]
      stderr_equal /.+/
      stdout_equal /\A(.*\n){#{@@f_mention_count}}\Z/
    end
  end
end
