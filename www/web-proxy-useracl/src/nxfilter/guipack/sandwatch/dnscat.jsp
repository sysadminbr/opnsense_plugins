<%@include file="../include/lib.jsp"%>
<%
DomainTestDao dao = new DomainTestDao();
DomainTestData data = dao.test(paramString("domain"));
out.println(data.category);
%>
