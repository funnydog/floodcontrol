deps = []
deps += dependency('freetype2', required : true, fallback : ['freetype2', 'freetype_dep'])
deps += dependency('glew', required : true, fallback : ['glew', 'glew_dep'])
deps += dependency('glfw3', required : true, fallback : ['glfw', 'glfw_dep'])
deps += dependency('glm', required : true, fallback : ['glm', 'glm_dep'])

srcs = [
  # application
  'application.cpp',
  'floodcontrol.cpp',

  # views
  'viewstack.cpp',
  'titleview.cpp',
  'gameview.cpp',
  'pipe.cpp',
  'fallingpipe.cpp',
  'rotatingpipe.cpp',
  'fadingpipe.cpp',
  'board.cpp',
  'scorezoom.cpp',
  'gameoverview.cpp',
  'pauseview.cpp',

  # graphics
  'camera.cpp',
  'eventqueue.cpp',
  'font.cpp',
  'rectangle.cpp',
  'rendertarget.cpp',
  'shader.cpp',
  'texture.cpp',
  'window.cpp',

  # utilities / third party
  'glcheck.cpp',
  'stb_image.cpp',
  'utility.cpp',
]

exe = executable(
  'floodcontrol',
  sources: srcs,
  dependencies: deps,
  install : true
)
