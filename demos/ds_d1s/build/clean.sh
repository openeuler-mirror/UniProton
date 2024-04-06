if [  -d "out" ]; then
  rm -rf out
fi

if [  -d "../include" ]; then
  rm -rf  ../include/*
fi

if [  -d "../libs" ]; then
  rm -rf ../libs/*
fi
