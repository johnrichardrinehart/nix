let
  user1 = "ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQCt7pYUfGHswe6BYtVSKRQJkHwm1Vvqia+dpspKSr/tXZ8SzDI9pRI8MbmTPhpi94yUkcYvVfgqWonN2ZBQaovpPmOF2llxmwAvOnzU1+HRRJGjFJEvAQkWVQbFTxwmIckzhNvVVr9X5e465AuQnIFreq8u4hU02MoVgNCEB/Oipj7jICDo3kksiB3UxZzswD1Fua/54ugR0jBCnJcwAojcCMPXz+6Pj80rogBt07yM8QSUus8b+MnAt8ufui7C4JU8vjFoRJXy05Q5BY2ShLNzPD5tOt4/Pkem2rgYtq2t1hV46VeztUBLHcZloFycSimiQCtJuUrnTxQWYiNIshDP";
in
{
  "secret1.age".publicKeys = [ user1 ];
}
