--
-- PostgreSQL database dump
--

-- Dumped from database version 8.4.12
-- Dumped by pg_dump version 9.0.1
-- Started on 2012-10-20 10:12:53 EST

SET statement_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = off;
SET check_function_bodies = false;
SET client_min_messages = warning;
SET escape_string_warning = off;

--
-- TOC entry 1875 (class 1262 OID 16387)
-- Name: membership; Type: DATABASE; Schema: -; Owner: -
--

CREATE DATABASE membership WITH TEMPLATE = template0 ENCODING = 'UTF8' LC_COLLATE = 'en_AU.UTF-8' LC_CTYPE = 'en_AU.UTF-8';


\connect membership

SET statement_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = off;
SET check_function_bodies = false;
SET client_min_messages = warning;
SET escape_string_warning = off;

SET search_path = public, pg_catalog;

SET default_with_oids = false;

--
-- TOC entry 1534 (class 1259 OID 16530)
-- Dependencies: 1830 1831 3
-- Name: Access_Control; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE "Access_Control" (
    "ID" integer NOT NULL,
    "Entity_ID" integer NOT NULL,
    "Access_Device_ID" integer NOT NULL,
    "Asset_ID" integer NOT NULL,
    "Active" boolean DEFAULT true NOT NULL,
    "isAuthoritative" boolean DEFAULT false NOT NULL
);


--
-- TOC entry 1533 (class 1259 OID 16528)
-- Dependencies: 1534 3
-- Name: Access_Control_ID_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE "Access_Control_ID_seq"
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 1877 (class 0 OID 0)
-- Dependencies: 1533
-- Name: Access_Control_ID_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE "Access_Control_ID_seq" OWNED BY "Access_Control"."ID";


--
-- TOC entry 1878 (class 0 OID 0)
-- Dependencies: 1533
-- Name: Access_Control_ID_seq; Type: SEQUENCE SET; Schema: public; Owner: -
--

SELECT pg_catalog.setval('"Access_Control_ID_seq"', 1, false);


--
-- TOC entry 1532 (class 1259 OID 16506)
-- Dependencies: 1828 3
-- Name: Access_Device; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE "Access_Device" (
    "ID" integer NOT NULL,
    "Key_Number" character varying(60) NOT NULL,
    "Issued_Date" date DEFAULT now() NOT NULL,
    "End_Date" date,
    "Notes" text,
    "Access_Device_Type_ID" integer NOT NULL
);


--
-- TOC entry 1531 (class 1259 OID 16504)
-- Dependencies: 3 1532
-- Name: Access_Device_Id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE "Access_Device_Id_seq"
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 1879 (class 0 OID 0)
-- Dependencies: 1531
-- Name: Access_Device_Id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE "Access_Device_Id_seq" OWNED BY "Access_Device"."ID";


--
-- TOC entry 1880 (class 0 OID 0)
-- Dependencies: 1531
-- Name: Access_Device_Id_seq; Type: SEQUENCE SET; Schema: public; Owner: -
--

SELECT pg_catalog.setval('"Access_Device_Id_seq"', 1, false);


--
-- TOC entry 1530 (class 1259 OID 16498)
-- Dependencies: 3
-- Name: Access_Device_Type; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE "Access_Device_Type" (
    "ID" integer NOT NULL,
    "Key_Type" character varying(15) NOT NULL
);


--
-- TOC entry 1536 (class 1259 OID 16554)
-- Dependencies: 1833 1834 3
-- Name: Access_History; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE "Access_History" (
    "ID" integer NOT NULL,
    "Entidy_ID" integer NOT NULL,
    "Access_Point_ID" integer NOT NULL,
    "Access_Device_ID" integer NOT NULL,
    "Authorised" boolean DEFAULT false NOT NULL,
    "Notes" character varying(50),
    "Log_Time" timestamp without time zone DEFAULT now() NOT NULL
);


--
-- TOC entry 1535 (class 1259 OID 16552)
-- Dependencies: 3 1536
-- Name: Access_History_ID_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE "Access_History_ID_seq"
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 1881 (class 0 OID 0)
-- Dependencies: 1535
-- Name: Access_History_ID_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE "Access_History_ID_seq" OWNED BY "Access_History"."ID";


--
-- TOC entry 1882 (class 0 OID 0)
-- Dependencies: 1535
-- Name: Access_History_ID_seq; Type: SEQUENCE SET; Schema: public; Owner: -
--

SELECT pg_catalog.setval('"Access_History_ID_seq"', 1, false);


--
-- TOC entry 1528 (class 1259 OID 16480)
-- Dependencies: 1820 1821 1822 1823 1824 1825 3
-- Name: Access_Point; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE "Access_Point" (
    "ID" integer NOT NULL,
    "Asset_ID" integer NOT NULL,
    "Device_Name" character varying(15) NOT NULL,
    "Device_Mac" character varying(18) DEFAULT '00:00:00:00:00:00'::character varying NOT NULL,
    "Device_Ip" character varying(20) DEFAULT '0.0.0.0'::character varying NOT NULL,
    salt character varying(15) DEFAULT 'default_salt'::character varying NOT NULL,
    "Code_Version" character varying(15) DEFAULT '0.0'::character varying NOT NULL,
    "Active" boolean DEFAULT true NOT NULL,
    "Heart_Beat" timestamp without time zone[],
    "isListener" boolean DEFAULT false NOT NULL
);


--
-- TOC entry 1527 (class 1259 OID 16478)
-- Dependencies: 3 1528
-- Name: Access_Point_Id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE "Access_Point_Id_seq"
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 1883 (class 0 OID 0)
-- Dependencies: 1527
-- Name: Access_Point_Id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE "Access_Point_Id_seq" OWNED BY "Access_Point"."ID";


--
-- TOC entry 1884 (class 0 OID 0)
-- Dependencies: 1527
-- Name: Access_Point_Id_seq; Type: SEQUENCE SET; Schema: public; Owner: -
--

SELECT pg_catalog.setval('"Access_Point_Id_seq"', 1, false);


--
-- TOC entry 1525 (class 1259 OID 16459)
-- Dependencies: 1818 3
-- Name: Asset; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE "Asset" (
    "ID" integer NOT NULL,
    "Friendly_Name" character varying(255) NOT NULL,
    "Purchase_Price" money,
    "Purchase_Date" date NOT NULL,
    "Decommission_Date" date,
    "Global_Allow" boolean DEFAULT false NOT NULL,
    "Notes" text,
    "Asset_Type_ID" integer NOT NULL
);


--
-- TOC entry 1524 (class 1259 OID 16457)
-- Dependencies: 3 1525
-- Name: Asset_Id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE "Asset_Id_seq"
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 1885 (class 0 OID 0)
-- Dependencies: 1524
-- Name: Asset_Id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE "Asset_Id_seq" OWNED BY "Asset"."ID";


--
-- TOC entry 1886 (class 0 OID 0)
-- Dependencies: 1524
-- Name: Asset_Id_seq; Type: SEQUENCE SET; Schema: public; Owner: -
--

SELECT pg_catalog.setval('"Asset_Id_seq"', 1, false);


--
-- TOC entry 1521 (class 1259 OID 16415)
-- Dependencies: 3
-- Name: Asset_Type; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE "Asset_Type" (
    "ID" integer NOT NULL,
    "Asset_Type" character varying(15)
);


--
-- TOC entry 1520 (class 1259 OID 16413)
-- Dependencies: 1521 3
-- Name: Asset_Type_Id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE "Asset_Type_Id_seq"
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 1887 (class 0 OID 0)
-- Dependencies: 1520
-- Name: Asset_Type_Id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE "Asset_Type_Id_seq" OWNED BY "Asset_Type"."ID";


--
-- TOC entry 1888 (class 0 OID 0)
-- Dependencies: 1520
-- Name: Asset_Type_Id_seq; Type: SEQUENCE SET; Schema: public; Owner: -
--

SELECT pg_catalog.setval('"Asset_Type_Id_seq"', 2, true);


--
-- TOC entry 1526 (class 1259 OID 16474)
-- Dependencies: 3
-- Name: Entity_Id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE "Entity_Id_seq"
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 1889 (class 0 OID 0)
-- Dependencies: 1526
-- Name: Entity_Id_seq; Type: SEQUENCE SET; Schema: public; Owner: -
--

SELECT pg_catalog.setval('"Entity_Id_seq"', 1, false);


--
-- TOC entry 1519 (class 1259 OID 16391)
-- Dependencies: 1814 3
-- Name: Entity; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE "Entity" (
    "ID" integer DEFAULT nextval('"Entity_Id_seq"'::regclass) NOT NULL,
    "F_Name" character varying(255) NOT NULL,
    "L_Name" character varying(255),
    "N_Name" character varying(50),
    "Phone" character varying(10),
    "Addr1" character varying(255),
    "Addr2" character varying(255),
    "Zip_Code" integer,
    "Entity_Type_ID" integer NOT NULL
);


--
-- TOC entry 1523 (class 1259 OID 16428)
-- Dependencies: 3
-- Name: Entity_Type; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE "Entity_Type" (
    "ID" integer NOT NULL,
    "Entity_Type" character varying(15)
);


--
-- TOC entry 1522 (class 1259 OID 16426)
-- Dependencies: 1523 3
-- Name: Entity_Type_Id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE "Entity_Type_Id_seq"
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 1890 (class 0 OID 0)
-- Dependencies: 1522
-- Name: Entity_Type_Id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE "Entity_Type_Id_seq" OWNED BY "Entity_Type"."ID";


--
-- TOC entry 1891 (class 0 OID 0)
-- Dependencies: 1522
-- Name: Entity_Type_Id_seq; Type: SEQUENCE SET; Schema: public; Owner: -
--

SELECT pg_catalog.setval('"Entity_Type_Id_seq"', 2, true);


--
-- TOC entry 1529 (class 1259 OID 16496)
-- Dependencies: 1530 3
-- Name: Key_Type_Id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE "Key_Type_Id_seq"
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 1892 (class 0 OID 0)
-- Dependencies: 1529
-- Name: Key_Type_Id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE "Key_Type_Id_seq" OWNED BY "Access_Device_Type"."ID";


--
-- TOC entry 1893 (class 0 OID 0)
-- Dependencies: 1529
-- Name: Key_Type_Id_seq; Type: SEQUENCE SET; Schema: public; Owner: -
--

SELECT pg_catalog.setval('"Key_Type_Id_seq"', 3, true);


--
-- TOC entry 1829 (class 2604 OID 16533)
-- Dependencies: 1534 1533 1534
-- Name: ID; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE "Access_Control" ALTER COLUMN "ID" SET DEFAULT nextval('"Access_Control_ID_seq"'::regclass);


--
-- TOC entry 1827 (class 2604 OID 16509)
-- Dependencies: 1532 1531 1532
-- Name: ID; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE "Access_Device" ALTER COLUMN "ID" SET DEFAULT nextval('"Access_Device_Id_seq"'::regclass);


--
-- TOC entry 1826 (class 2604 OID 16501)
-- Dependencies: 1530 1529 1530
-- Name: ID; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE "Access_Device_Type" ALTER COLUMN "ID" SET DEFAULT nextval('"Key_Type_Id_seq"'::regclass);


--
-- TOC entry 1832 (class 2604 OID 16557)
-- Dependencies: 1535 1536 1536
-- Name: ID; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE "Access_History" ALTER COLUMN "ID" SET DEFAULT nextval('"Access_History_ID_seq"'::regclass);


--
-- TOC entry 1819 (class 2604 OID 16483)
-- Dependencies: 1527 1528 1528
-- Name: ID; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE "Access_Point" ALTER COLUMN "ID" SET DEFAULT nextval('"Access_Point_Id_seq"'::regclass);


--
-- TOC entry 1817 (class 2604 OID 16462)
-- Dependencies: 1524 1525 1525
-- Name: ID; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE "Asset" ALTER COLUMN "ID" SET DEFAULT nextval('"Asset_Id_seq"'::regclass);


--
-- TOC entry 1815 (class 2604 OID 16418)
-- Dependencies: 1521 1520 1521
-- Name: ID; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE "Asset_Type" ALTER COLUMN "ID" SET DEFAULT nextval('"Asset_Type_Id_seq"'::regclass);


--
-- TOC entry 1816 (class 2604 OID 16431)
-- Dependencies: 1522 1523 1523
-- Name: ID; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE "Entity_Type" ALTER COLUMN "ID" SET DEFAULT nextval('"Entity_Type_Id_seq"'::regclass);


--
-- TOC entry 1871 (class 0 OID 16530)
-- Dependencies: 1534
-- Data for Name: Access_Control; Type: TABLE DATA; Schema: public; Owner: -
--

COPY "Access_Control" ("ID", "Entity_ID", "Access_Device_ID", "Asset_ID", "Active", "isAuthoritative") FROM stdin;
\.


--
-- TOC entry 1870 (class 0 OID 16506)
-- Dependencies: 1532
-- Data for Name: Access_Device; Type: TABLE DATA; Schema: public; Owner: -
--

COPY "Access_Device" ("ID", "Key_Number", "Issued_Date", "End_Date", "Notes", "Access_Device_Type_ID") FROM stdin;
\.


--
-- TOC entry 1869 (class 0 OID 16498)
-- Dependencies: 1530
-- Data for Name: Access_Device_Type; Type: TABLE DATA; Schema: public; Owner: -
--

COPY "Access_Device_Type" ("ID", "Key_Type") FROM stdin;
1	rfid_card
2	rfid_fob
3	lock_key
\.


--
-- TOC entry 1872 (class 0 OID 16554)
-- Dependencies: 1536
-- Data for Name: Access_History; Type: TABLE DATA; Schema: public; Owner: -
--

COPY "Access_History" ("ID", "Entidy_ID", "Access_Point_ID", "Access_Device_ID", "Authorised", "Notes", "Log_Time") FROM stdin;
\.


--
-- TOC entry 1868 (class 0 OID 16480)
-- Dependencies: 1528
-- Data for Name: Access_Point; Type: TABLE DATA; Schema: public; Owner: -
--

COPY "Access_Point" ("ID", "Asset_ID", "Device_Name", "Device_Mac", "Device_Ip", salt, "Code_Version", "Active", "Heart_Beat", "isListener") FROM stdin;
\.


--
-- TOC entry 1867 (class 0 OID 16459)
-- Dependencies: 1525
-- Data for Name: Asset; Type: TABLE DATA; Schema: public; Owner: -
--

COPY "Asset" ("ID", "Friendly_Name", "Purchase_Price", "Purchase_Date", "Decommission_Date", "Global_Allow", "Notes", "Asset_Type_ID") FROM stdin;
\.


--
-- TOC entry 1865 (class 0 OID 16415)
-- Dependencies: 1521
-- Data for Name: Asset_Type; Type: TABLE DATA; Schema: public; Owner: -
--

COPY "Asset_Type" ("ID", "Asset_Type") FROM stdin;
2	door
1	item
\.


--
-- TOC entry 1864 (class 0 OID 16391)
-- Dependencies: 1519
-- Data for Name: Entity; Type: TABLE DATA; Schema: public; Owner: -
--

COPY "Entity" ("ID", "F_Name", "L_Name", "N_Name", "Phone", "Addr1", "Addr2", "Zip_Code", "Entity_Type_ID") FROM stdin;
\.


--
-- TOC entry 1866 (class 0 OID 16428)
-- Dependencies: 1523
-- Data for Name: Entity_Type; Type: TABLE DATA; Schema: public; Owner: -
--

COPY "Entity_Type" ("ID", "Entity_Type") FROM stdin;
1	individual
2	corporation
\.


--
-- TOC entry 1851 (class 2606 OID 16536)
-- Dependencies: 1534 1534
-- Name: Access_Control_PK; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY "Access_Control"
    ADD CONSTRAINT "Access_Control_PK" PRIMARY KEY ("ID");


--
-- TOC entry 1848 (class 2606 OID 16514)
-- Dependencies: 1532 1532
-- Name: Access_Device_PK; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY "Access_Device"
    ADD CONSTRAINT "Access_Device_PK" PRIMARY KEY ("ID");


--
-- TOC entry 1853 (class 2606 OID 16561)
-- Dependencies: 1536 1536
-- Name: Access_History_PK; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY "Access_History"
    ADD CONSTRAINT "Access_History_PK" PRIMARY KEY ("ID");


--
-- TOC entry 1844 (class 2606 OID 16490)
-- Dependencies: 1528 1528
-- Name: Access_Point_PK; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY "Access_Point"
    ADD CONSTRAINT "Access_Point_PK" PRIMARY KEY ("ID");


--
-- TOC entry 1842 (class 2606 OID 16468)
-- Dependencies: 1525 1525
-- Name: Asset_PK; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY "Asset"
    ADD CONSTRAINT "Asset_PK" PRIMARY KEY ("ID");


--
-- TOC entry 1838 (class 2606 OID 16420)
-- Dependencies: 1521 1521
-- Name: Asset_Type_PK; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY "Asset_Type"
    ADD CONSTRAINT "Asset_Type_PK" PRIMARY KEY ("ID");


--
-- TOC entry 1836 (class 2606 OID 16399)
-- Dependencies: 1519 1519
-- Name: Entity_PK; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY "Entity"
    ADD CONSTRAINT "Entity_PK" PRIMARY KEY ("ID");


--
-- TOC entry 1840 (class 2606 OID 16433)
-- Dependencies: 1523 1523
-- Name: Entity_Type_PK; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY "Entity_Type"
    ADD CONSTRAINT "Entity_Type_PK" PRIMARY KEY ("ID");


--
-- TOC entry 1846 (class 2606 OID 16503)
-- Dependencies: 1530 1530
-- Name: Key_Type_PK; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY "Access_Device_Type"
    ADD CONSTRAINT "Key_Type_PK" PRIMARY KEY ("ID");


--
-- TOC entry 1849 (class 1259 OID 16586)
-- Dependencies: 1532
-- Name: fki_Access_Device_Type_FK; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX "fki_Access_Device_Type_FK" ON "Access_Device" USING btree ("Access_Device_Type_ID");


--
-- TOC entry 1859 (class 2606 OID 16542)
-- Dependencies: 1847 1532 1534
-- Name: Access_Device_FK; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY "Access_Control"
    ADD CONSTRAINT "Access_Device_FK" FOREIGN KEY ("Access_Device_ID") REFERENCES "Access_Device"("ID") ON DELETE RESTRICT;


--
-- TOC entry 1862 (class 2606 OID 16567)
-- Dependencies: 1847 1532 1536
-- Name: Access_Device_FK; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY "Access_History"
    ADD CONSTRAINT "Access_Device_FK" FOREIGN KEY ("Access_Device_ID") REFERENCES "Access_Device"("ID") ON DELETE RESTRICT;


--
-- TOC entry 1857 (class 2606 OID 16581)
-- Dependencies: 1530 1845 1532
-- Name: Access_Device_Type_FK; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY "Access_Device"
    ADD CONSTRAINT "Access_Device_Type_FK" FOREIGN KEY ("Access_Device_Type_ID") REFERENCES "Access_Device_Type"("ID") ON DELETE RESTRICT;


--
-- TOC entry 1863 (class 2606 OID 16572)
-- Dependencies: 1528 1843 1536
-- Name: Access_Point_FK; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY "Access_History"
    ADD CONSTRAINT "Access_Point_FK" FOREIGN KEY ("Access_Point_ID") REFERENCES "Access_Point"("ID") ON DELETE RESTRICT;


--
-- TOC entry 1856 (class 2606 OID 16491)
-- Dependencies: 1528 1841 1525
-- Name: Asset_FK; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY "Access_Point"
    ADD CONSTRAINT "Asset_FK" FOREIGN KEY ("Asset_ID") REFERENCES "Asset"("ID") ON DELETE RESTRICT;


--
-- TOC entry 1860 (class 2606 OID 16547)
-- Dependencies: 1841 1525 1534
-- Name: Asset_FK; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY "Access_Control"
    ADD CONSTRAINT "Asset_FK" FOREIGN KEY ("Asset_ID") REFERENCES "Asset"("ID") ON DELETE RESTRICT;


--
-- TOC entry 1855 (class 2606 OID 16469)
-- Dependencies: 1525 1521 1837
-- Name: Asset_Type; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY "Asset"
    ADD CONSTRAINT "Asset_Type" FOREIGN KEY ("Asset_Type_ID") REFERENCES "Asset_Type"("ID") MATCH FULL ON DELETE RESTRICT;


--
-- TOC entry 1858 (class 2606 OID 16537)
-- Dependencies: 1534 1519 1835
-- Name: Entity_FK; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY "Access_Control"
    ADD CONSTRAINT "Entity_FK" FOREIGN KEY ("Entity_ID") REFERENCES "Entity"("ID") ON DELETE RESTRICT;


--
-- TOC entry 1861 (class 2606 OID 16562)
-- Dependencies: 1536 1835 1519
-- Name: Entity_FK; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY "Access_History"
    ADD CONSTRAINT "Entity_FK" FOREIGN KEY ("Entidy_ID") REFERENCES "Entity"("ID") ON DELETE RESTRICT;


--
-- TOC entry 1854 (class 2606 OID 16452)
-- Dependencies: 1839 1519 1523
-- Name: Entity_Type_FK; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY "Entity"
    ADD CONSTRAINT "Entity_Type_FK" FOREIGN KEY ("Entity_Type_ID") REFERENCES "Entity_Type"("ID") ON DELETE RESTRICT;


-- Completed on 2012-10-20 10:12:54 EST

--
-- PostgreSQL database dump complete
--

