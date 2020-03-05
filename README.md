# Socket-library
C++ Socket wrapper



## Usage

```c++

int main()
{
  Socket socket(ConnectionDetails("127.0.0.1", 80, ConnectionDetails::type::IPV4), Socket::type::TCP);

  if(!socket.bind() || !socket.listen())
      throw std::runtime_error("Error starting server");

  while(true)
  {
     Socket client = socket.accept();

     if(!client){
	continue;
     }
     printf("%s connected!\n", client.m_details.m_ip.c_str());
  }
}


```