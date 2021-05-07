require 'mkmf'
$defs.push('-Wall') if enable_config('all-warnings')
create_makefile('digest/xxhash')
File.write('Makefile', 'V = 1', mode: 'a') if enable_config('verbose-mode')
