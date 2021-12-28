require 'bundler/gem_tasks'
require 'rake/extensiontask'
require 'rake/testtask'

# clean, clobber, compile, and compile:digest/xxhash
Rake::ExtensionTask.new('digest/xxhash', Bundler::GemHelper.gemspec)

# compile_lazy
task :compile_lazy do
  Rake::Task[:compile].invoke
end.instance_eval do
  def needed?
    !File.exist?("lib/digest/xxhash.so")
  end
end

# test
Rake::TestTask.new(:test => :compile_lazy) do |t|
  t.test_files = FileList['test/test.rb']
  t.verbose = true
end

# clean
task :clean do
  list = FileList.new('test/*.tmp', 'test/*.temp')
  rm_f list unless list.empty?
end

# default
task :default => [:compile, :test]

# Run `rake --tasks` for a list of tasks.
