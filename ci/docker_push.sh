#!/bin/bash
set -ev
want_push='false'
for branch in "master" "1.0.0" "1.1.0"
do
   if [ "$TRAVIS_BRANCH" == "$branch" ]
   then
       want_push='true'
   fi
done
# if [ "$TRAVIS_PULL_REQUEST" == "false" ] && [ "$want_push" == "true" ] && [ "$TEST_TYPE" == "normal" ]
if [ 0 == 0 ]
then
    # docker login -e $DOCKER_EMAIL -u $DOCKER_USERNAME -p $DOCKER_PASSWORD
    export TAG=`if [ "$TRAVIS_BRANCH" == "master" ]; then echo "latest"; else echo $TRAVIS_BRANCH; fi`
    echo "TAG is $TAG"
    docker build -f ci/Dockerfile-envoy-image -t lyft/envoy:$TAG
    docker images
    # docker push lyft/envoy:$TAG
else
    echo 'Ignoring PR branch for docker push.'
fi