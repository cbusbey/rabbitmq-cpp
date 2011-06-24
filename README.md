# rabbitmq-cpp

a nice c++ wrapper for rabbitmq-c <https://github.com/rabbitmq/rabbitmq-c>

# BUILD

You will need the rabbitmq-c, and rabbitmq-codegen libs. See the rabbitmq-c README for other pre-reqs.  

Build and checkout in the project root.  

    git clone git://github.com/rabbitmq/rabbitmq-c.git
    git clone git://github.com/rabbitmq/rabbitmq-codegen.git
    pushd rabbitmq-c
    autoreconf -i
    ./configure
    make

Once deps are built...

    ./bootstrap
    ./configure
    make

# EXAMPLES
    
    see examples/
