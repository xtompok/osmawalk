# copied from http://www.cs.toronto.edu/~rdanek/csc148h_09
class BinarySearchTree:
	def __init__(self):
		""" create an empty binary search tree """
		self.root = None
		
	def put(self, line, lon):
		""" add a new mapping between key and value to the BST """
		if self.root:
			return self.root.put(line,lon)
		else:
			self.root = TreeNode(line)
			return (None,None)

	def get(self, line, lon):
		""" retrieve the value associated with the given key """
		if self.root:
			return self.root.get(line,lon)
		else:
			return None
		
	def delete(self, line, lon):
		""" delete the node with the given key if it exists """
		if self.root:
			self.root = self.root.delete(line, lon)
		else:
			print "Tree empty!"
			
			
class TreeNode:
	def __init__(self, line):
		self.line = line
		self.left = None
		self.right = None 
		self.parent = None
			
	def put(self, line, lon):
		""" add a new mapping between key and value in the tree """
		if self.line.compare(lon) == line.compare(lon):
		#	print "Warning, identical lines!"
			return (self.getPrev(),self.getNext())
		elif self.line.compare(lon) > line.compare(lon):			# key belongs in left subtree 
			if self.left:
				return self.left.put(line,lon)
			else:					   # left subtree is empty
				self.left = TreeNode(line)
				self.left.parent = self
				return (self.left.getPrev(),self.left.getNext())
		else:						   # key belongs in right subtree 
			if self.right:
				return self.right.put(line,lon)
			else:					   # right subtree is empty
				self.right = TreeNode(line)
				self.right.parent = self
				return (self.right.getPrev(),self.right.getNext())
				
	def getPrev(self):
		if self.left:
			return self._findMax(self)[1].line
		if not self.parent:
			return None

		memself = self
		self = self.parent
		while (self.parent and self.parent.left==self):
			self = self.parent

		if self.right==memself:
			return self.line
		return None

	def getNext(self):
		if self.right:
			return self._findMin(self)[1].line
		if not self.parent:
			return None

		memself = self
		self = self.parent
		while (self.parent and self.parent.right==self):
			self = self.parent

		if self.left==memself:
			return self.line
		return None
		
		
		
	def get(self, line,lon):
		""" get the value associated with the key """
		if self.line == line:
			return self.line
		
		if self.line.compare(lon) > line.compare(lon):	   # key should be in the left subtree
			if self.left:
				return self.left.get(line,lon)
			else:
				return None
		else:						   # key should be in the right subtree
			if self.right:
				return self.right.get(line,lon)
			else:
				return None
			
	def delete(self, line,lon):
		""" delete the node with the given key and return the 
		root node of the tree """
			
		if self.line.compare(lon) == line.compare(lon):
			if self.line != line:
				pass
			#   print "Warning, non equal objects!"	
			# found the node we need to delete
			
			if self.right and self.left: 
				
				# get the successor node and its parent 
				[psucc, succ] = self.right._findMin(self)
				
				# splice out the successor
				# (we need the parent to do this) 
				
				if psucc.left == succ:
					psucc.left = succ.right
					if succ.right:
						psucc.left.parent = psucc
				else:
					psucc.right = succ.right
					if succ.right:
						psucc.right.parent = succ.right
								
				# reset the left and right children of the successor
				
				succ.left = self.left
				if self.left:
					succ.left.parent = succ
				succ.right = self.right
				if self.right:
					succ.right.parent = succ
				
				return succ				
				
			else:
				# "easier" case
				if self.left:
					return self.left	# promote the left subtree
				else:
					return self.right   # promote the right subtree 
		else:
			if self.line.compare(lon) > line.compare(lon):		  # key should be in the left subtree
				if self.left:
					self.left = self.left.delete(line,lon)
					if self.left:
						self.left.parent = self
				# else the key is not in the tree 
					
			else:					   # key should be in the right subtree
				if self.right:
					self.right = self.right.delete(line,lon)
					if self.right:
						self.right.parent = self
					
		return self
	
	def _findMin(self, parent):
		""" return the minimum node in the current tree and its parent """

		# we use an ugly trick: the parent node is passed in as an argument
		# so that eventually when the leftmost child is reached, the 
		# call can return both the parent to the successor and the successor
		
		if self.left:
			return self.left._findMin(self)
		else:
			return [parent, self]

	def _findMax(self, parent):
		""" return the minimum node in the current tree and its parent """

		# we use an ugly trick: the parent node is passed in as an argument
		# so that eventually when the leftmost child is reached, the 
		# call can return both the parent to the successor and the successor
		
		if self.right:
			return self.right._findMin(self)
		else:
			return [parent, self]
