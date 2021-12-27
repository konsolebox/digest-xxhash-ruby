require 'bundler/gem_tasks'
require 'rake/extensiontask'
require 'rake/testtask'

# clean, clobber, compile, and compile:digest/xxhash
Rake::ExtensionTask.new('digest/xxhash', Bundler::GemHelper.gemspec)

# install
Rake::Task[:install].clear
task :install => :build do
  name = Bundler::GemHelper.gemspec.name
  pkg_dir = File.join(Bundler::GemHelper.instance.base, "pkg")
  built_gem = Dir.chdir(pkg_dir){ Dir.glob("#{name}-*.gem").sort_by{ |f| File.mtime(f) }.last }
  gem_command = (ENV["GEM_COMMAND"].shellsplit rescue nil) || ["gem"]
  options = ARGV.select{ |e| e =~ /\A--?/ }
  Process.wait spawn(*gem_command, "install", File.join(pkg_dir, built_gem), *options)
end

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
